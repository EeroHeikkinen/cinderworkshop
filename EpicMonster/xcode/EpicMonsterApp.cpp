
/*
 Copyright (C) 2011-2012 Gabor Papp
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Cinder.h"
#include "cinder/ImageIo.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"
#include "cinder/Quaternion.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/Utilities.h"
#include "cinder/Ray.h"
#include "cinder/app/App.h"
#include "cinder/Timer.h"

#include "AssimpLoader.h"
#include "PingPongFbo.h"
#include "Resources.h"
#include "OscListener.h"
#include "cinderSyphon.h"

#include "boost/assign.hpp"
using namespace std;
using namespace boost::assign;

#define SIDE 2048

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace mndl;

map<string, string> boneMap = map_list_of
    //("/Head","Head")
    ("/Left_Elbow","Left_Elbow")
    /*("/Right_Elbow","Right_Elbow")
    //("/Left_Hand","Left_Hand")
    //("/Right_Hand","Right_Hand")
    ("/Neck","Neck")
    ("/Torso","MonsuPos")
    ("/Left_Hip","Left_Hip")
    ("/Right_Hip","Right_Hip")
    ("/Left_Knee","Left_Knee")
    ("/Right_Knee","Right_Knee")
    ("/Left_Shoulder","Left_Shoulder")
    ("/Right_Shoulder","Right_Shoulder")
    ("/Left_Foot","Left_Foot")
    ("/Right_Foot","Right_Foot")*/;

class EpicMonsterApp : public AppBasic
{
public:
    void prepareSettings( Settings *settings );
    void setup();
    
    void resize( ResizeEvent event );
    void mouseDown( MouseEvent event );
    void mouseMove( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void keyDown (KeyEvent event);
    void computeAttractorPosition();
    void flockToBody();
    
    void update();
    void draw();
    
private:
    assimp::AssimpLoader mAssimpLoader;
    
    Vec2f mMousePos;
    
    MayaCamUI mMayaCam;
    
    params::InterfaceGl mParams;
    void setupParams();
    
    bool mEnableWireframe;
    bool mEnableDebugTexture;
    float mTimerSlower;
    float mFps;
    
    vector< string > mNodeNames;
    int mNodeIndex;
    bool mNoBones;
    vector< Quatf > mNodeOrientations;
    vector< Vec3f > mNodePositions;
    float mParIteration;
    syphonServer mScreenSyphon;
    
    PingPongFbo mPPFbo;
    gl::VboMesh mVboMesh;
    gl::GlslProg mParticlesShader, mDisplacementShader, mBakeShader;
    Vec3f mAttractor;
    bool mStep;
    Vec3f mParTexOffset;
    Vec3f mFallDirection;
    Vec3f n;
    
    unsigned int limit = 0;
    float triangles = 0;
    gl::Texture mNormalMap;
    void setupPingPongFbo();
    void setupVBO();
    float    mNormalScale = 0.01;
    osc::Listener listener;
    
    cinder::Timer timer;
};


void EpicMonsterApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 800 );
}

void EpicMonsterApp::flockToBody() {
    if(timer.getSeconds() > mTimerSlower) {
        timer.stop();
        timer.start();
        
        gl::setMatricesWindow( mPPFbo.getSize(), false ); // false to prevent vertical flipping;
    
        mPPFbo.updateBind();
    
        mBakeShader.bind();
        mBakeShader.uniform( "positions", 0 );
        mBakeShader.uniform( "scale", mNormalScale );
        mBakeShader.uniform( "velocities", 1 );
        mBakeShader.uniform( "n", n);
        mBakeShader.uniform( "attractorPos", mAttractor);
        
        mBakeShader.uniform( "offset", Vec3f(mParTexOffset.x, mParTexOffset.y, mParIteration));
    
        
        glDisable(GL_CULL_FACE);
        mAssimpLoader.draw();
        mBakeShader.unbind();
    
        mPPFbo.updateUnbind();
        mPPFbo.swap();
        mParIteration+= triangles;
        if(mParIteration > (6*n.x*n.x))
            mParIteration = 0;
        // TODO: baketuksessa overflowaa tyylikkäästi takaisin alkuun, jos menee yli
    }

}

void EpicMonsterApp::computeAttractorPosition()
{
    // The attractor is positioned at the intersection of a ray
    // from the mouse to a plane perpendicular to the camera.
    float t = 0;
    Vec3f right, up;
    mMayaCam.getCamera().getBillboardVectors(&right, &up);
    CameraPersp cam = mMayaCam.getCamera();
    float u = mMousePos.x / (float) getWindowWidth();
    float v = mMousePos.y / (float) getWindowHeight();
    
    Ray ray = cam.generateRay(u , 1.0f - v, cam.getAspectRatio() );
    if (ray.calcPlaneIntersection(Vec3f(0.0f,0.0f,0.0f), right.cross(up), &t)) {
        mAttractor.set(ray.calcPosition(t));
    }
}

void EpicMonsterApp::setup()
{
    mScreenSyphon.setName("Epic Monster Demo");
    
    mFallDirection = Vec3f(0.0,-1.0,0.0);
    listener.setup(7000);
    
    // Slows down particle pulses
    mTimerSlower = 0.05;
    
    // Number of iterations for particle drawing to texture
    n = Vec3f(128, 128, 0);
    
    // Where texture baking shader starts drawing (obsolete)
    mParTexOffset = Vec3f(-1.0, 1.0, 0.0);
    timer = cinder::Timer(true);
    mStep = true;
    try {
        // Multiple render targets shader updates the positions/velocities
        mParticlesShader = gl::GlslProg( ci::app::loadResource( PASSTHROUGH_VERT ), ci::app::loadResource( PARTICLES_FRAG ));
        // Vertex displacement shader
        mDisplacementShader = gl::GlslProg( ci::app::loadResource( VERTEXDISPLACEMENT_VERT ), ci::app::loadResource( VERTEXDISPLACEMENT_FRAG ));
        
        mBakeShader = gl::GlslProg( ci::app::loadResource( BAKE_VERT ), ci::app::loadResource( BAKE_FRAG ));
    }
    catch( ci::gl::GlslProgCompileExc &exc ) {
        std::cout << "Shader compile error: " << endl;
        std::cout << exc.what();
    }
    catch( const std::exception& ex ) {
        std::cout << "Unable to load shader" << endl;
    }
    
    setupPingPongFbo();
    // THE VBO HAS TO BE DRAWN AFTER FBO!
    setupVBO();
    
    // End of Particle setup
    
    try {
    mAssimpLoader = assimp::AssimpLoader( getAssetPath( "Monsu7b.dae" ) );
        
    }
    catch(const std::exception& ex ) {
        std::cout << "Model loading error: " << endl;
        std::cout << ex.what();
    }
    
    mNormalMap	= gl::Texture( loadImage( loadResource( RES_NORMAL ) ) );
	mAssimpLoader.enableSkinning();
    
	mNodeNames = mAssimpLoader.getNodeNames();
	if ( mNodeNames.empty () )
	{
		mNodeNames.push_back( "NO BONES!" );
		mNoBones = true;
	}
	else
	{
		mNoBones = false;
	}
    
	// query original node orientations from model
	mNodeOrientations.assign( mNodeNames.size(), Quatf() );
	if ( !mNoBones )
	{
		for ( size_t i = 0; i < mNodeOrientations.size(); ++i )
		{
			mNodeOrientations[ i ] = mAssimpLoader.getNodeOrientation( mNodeNames[ i ] );
		}
	}
    
    // query original node orientations from model
	mNodePositions.assign( mNodeNames.size(), Vec3f() );
	if ( !mNoBones )
	{
		for ( size_t i = 0; i < mNodePositions.size(); ++i )
		{
			mNodePositions[ i ] = mAssimpLoader.getNodePosition( mNodeNames[ i ] );
		}
	}
    
	mNodeIndex = 0;
    triangles = mAssimpLoader.totalTriangles();
	mEnableWireframe = false;
    mEnableDebugTexture = false;
    
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 300 ) );
    
	setupParams();
    
    CameraPersp cam;
	cam.setPerspective( 60, getWindowAspectRatio(), 0.1f, 1000.0f );
	cam.setEyePoint( Vec3f( 0, 1, 3 ) );
	cam.setCenterOfInterestPoint( Vec3f( 0, 0, 0 ) );
	mMayaCam.setCurrentCam( cam );
}


void EpicMonsterApp::setupPingPongFbo()
{
    float scale = 8.0f;
    // TODO: Test with more than 2 texture attachments
    Surface32f surfaces[2];
    // Position 2D texture array
    surfaces[0] = Surface32f( SIDE, SIDE, true);
    Surface32f::Iter pixelIter = surfaces[0].getIter();
    while( pixelIter.line() ) {
        while( pixelIter.pixel() ) {
            /* Initial particle positions are passed in as R,G,B
             float values. Alpha is used as particle invMass. */
            surfaces[0].setPixel(pixelIter.getPos(),
                                 ColorAf(scale*(Rand::randFloat()-0.5f),
                                         scale*(Rand::randFloat()-0.5f),
                                         scale*(Rand::randFloat()-0.5f),
                                         Rand::randFloat(0.2f, 1.0f) ) );
        }
    }
    
    //Velocity 2D texture array
    surfaces[1] = Surface32f( SIDE, SIDE, true);
    pixelIter = surfaces[1].getIter();
    while( pixelIter.line() ) {
        while( pixelIter.pixel() ) {
            /* Initial particle velocities are
             passed in as R,G,B float values. */
            surfaces[1].setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
        }
    }
    mPPFbo = PingPongFbo(surfaces);
}

void EpicMonsterApp::setupVBO(){
    /*  A dummy VboMesh the same size as the
     texture to keep the vertices on the GPU */
    int totalVertices = SIDE * SIDE;
    vector<Vec2f> texCoords;
    vector<uint32_t> indices;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    layout.setDynamicColorsRGBA();
    
    glPointSize(1.0f);
    mVboMesh = gl::VboMesh( totalVertices, totalVertices, layout, GL_POINTS);
    
    for( int x = 0; x < SIDE; ++x ) {
        for( int y = 0; y < SIDE; ++y ) {
            indices.push_back( x * SIDE + y );
            texCoords.push_back( Vec2f( x/(float)SIDE, y/(float)SIDE ) );
        }
    }
    
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
}

void EpicMonsterApp::update()
{
	static int lastNodeIndex = -1;
    
	if ( mNodeIndex != lastNodeIndex )
	{
		setupParams();
		lastNodeIndex = mNodeIndex;
	}
    
	if ( !mNoBones )
	{
		mAssimpLoader.setNodeOrientation( mNodeNames[ mNodeIndex ],
                                         mNodeOrientations[ mNodeIndex ] );
        
        mAssimpLoader.setNodePosition( mNodeNames[ mNodeIndex ],
                                         mNodePositions[ mNodeIndex ] );
	}
    
	mAssimpLoader.update();
    
	mFps = getAverageFps();
    
    
    computeAttractorPosition();
    
    if (mStep) {
        
        gl::setMatricesWindow( mPPFbo.getSize(), false ); // false to prevent vertical flipping
        gl::setViewport( mPPFbo.getBounds() );
        
        mPPFbo.updateBind();
        
        mParticlesShader.bind();
        mParticlesShader.uniform( "positions", 0 );
        mParticlesShader.uniform( "velocities", 1 );
        mParticlesShader.uniform( "attractorPos", mAttractor);
        mParticlesShader.uniform( "fallDirection", mFallDirection);
        

        gl::drawSolidRect(mPPFbo.getBounds());
        mParticlesShader.unbind();
        
        
        mPPFbo.updateUnbind();
        mPPFbo.swap();
    }
    
    //else {
        flockToBody();
    //}
     
     
    
    while (listener.hasWaitingMessages()) {
		osc::Message message;
		listener.getNextMessage(&message);
		
		console() << "New message received" << std::endl;
		console() << "Address: " << message.getAddress() << std::endl;
        
        if(boneMap.find(message.getAddress()) != boneMap.end()) {
           string bone = boneMap.at(message.getAddress());
            
            Vec3f pos = Vec3f(message.getArgAsFloat(0), message.getArgAsFloat(1), message.getArgAsFloat(2));
            Quatf ori = Quatf(message.getArgAsFloat(3), message.getArgAsFloat(4), message.getArgAsFloat(5), message.getArgAsFloat(6));
            mAssimpLoader.setNodePosition(bone, pos*0.2);
            mAssimpLoader.setNodeOrientation(bone, ori);
        }
		
	}
}


void EpicMonsterApp::setupParams()
{
    mParams.removeParam("Wireframe");
    mParams.removeParam("Nodes");
    mParams.removeParam("Rotation");
    mParams.removeParam("Fps");
    mParams.removeParam("Position");
    mParams.removeParam("iteration");
    
    //	mParams.clear();
    
	mParams.addParam( "Wireframe", &mEnableWireframe );
    	mParams.addParam( "Debug", &mEnableDebugTexture );
    	mParams.addParam( "Timer Slower", &mTimerSlower );
    	mParams.addParam( "Offset", &mParTexOffset );
        	mParams.addParam( "Fall Dir", &mFallDirection );
    mParams.addParam( "Triangles", &triangles );
    mParams.addParam( "iteration", &mParIteration );
    
    mParams.addParam( "n", &n );
    mParams.addParam( "normal map scale", &mNormalScale );
	//mParams.addSeparator();
    
	mParams.addParam( "Nodes", mNodeNames, &mNodeIndex, "", mNoBones );
	//mParams.addSeparator();
    
	if ( mNoBones )
		return;
    
	mParams.addParam( "Rotation", &mNodeOrientations[ mNodeIndex ], "opened=true" );
    mParams.addParam( "Position", &mNodePositions[ mNodeIndex ], "opened=true" );
	//mParams.addButton( "Reset current", bind( &SkinningApp::resetCurrentBone, this ) );
	//mParams.addButton( "Reset all", bind( &SkinningApp::resetAllBones, this ) );
    
	//mParams.addSeparator();
	mParams.addParam( "Fps", &mFps, "", true );
}


void EpicMonsterApp::draw()
{
	gl::clear( Color::black() );
    
	gl::setMatrices( mMayaCam.getCamera() );
    gl::setViewport( getWindowBounds() );
    
    
    
	gl::enableDepthWrite();
	gl::enableDepthRead();
    
	gl::color( Color::white() );
    
	if ( mEnableWireframe )
		gl::enableWireframe();
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setAmbient( Color::white() );
	light.setDiffuse( Color::white() );
	light.setSpecular( Color::white() );
	light.lookAt( Vec3f( 0, 0, 0 ), Vec3f( 0, 5, 0 ) );
	light.update( mMayaCam.getCamera() );
	light.enable();
    
	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );
    
    mAssimpLoader.draw();
    
    
    mNormalMap.bind( 2 );
    
    mPPFbo.bindTexture(0);
    mPPFbo.bindTexture(1);
    mDisplacementShader.bind();
    mDisplacementShader.uniform("displacementMap", 0 );
    mDisplacementShader.uniform("velocityMap", 1);
    mDisplacementShader.uniform("normalMap", 2);
        mDisplacementShader.uniform("fallDirection", mFallDirection);
    gl::draw( mVboMesh );
    mDisplacementShader.unbind();
    mPPFbo.unbindTexture();
    
    if(mEnableDebugTexture) {
        gl::Texture tex = mPPFbo.getTexture(0);
        //console() << getWindowBounds();
        gl::draw(tex, Rectf(-5.0f, -5.0f, 5.0f, 5.0f));
    }
    
    
	
    //gl::setMatricesWindow(getWindowSize());
    gl::drawString( toString( SIDE*SIDE ) + " vertices", Vec2f(32.0f, 32.0f));
        gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
    
	gl::disable( GL_LIGHTING );
    
	if ( mEnableWireframe )
		gl::disableWireframe();
    
    mScreenSyphon.publishScreen(); //publish the screen
    
	params::InterfaceGl::draw();
}

void EpicMonsterApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void EpicMonsterApp::keyDown(KeyEvent event) {
    if( event.getChar() == 'r' ) {
        mPPFbo.reloadTextures();
    } else if (event.getChar() == ' ') {
        mStep = !mStep;
    } else if (event.getChar() == 'k') {
        limit++;
    } else if (event.getChar() == 'j') {
        limit--;
    } else if (event.getChar() == 'f') {
        setFullScreen( !isFullScreen() );
    }
}

void EpicMonsterApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}


void EpicMonsterApp::mouseMove( MouseEvent event )
{
    mMousePos.set(event.getPos());
}

void EpicMonsterApp::resize( ResizeEvent event )
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

CINDER_APP_BASIC( EpicMonsterApp, RendererGl(0) )
