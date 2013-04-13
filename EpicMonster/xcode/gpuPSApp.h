/*
 Copyright (C) 2011-2012 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

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

#include "PingPongFbo.h"
#include "Resources.h"
#include "gpuPSApp.h"

namespace gpups {

    const int SIDE = 2048;
    
    using namespace ci;
    using namespace ci::app;
    using namespace std;
    
    class gpuPSApp {
    private:
        PingPongFbo mPPFbo;
        gl::VboMesh mVboMesh;
        gl::GlslProg mParticlesShader, mDisplacementShader;
        Vec3f mAttractor;
        Vec2f mMousePos;
        bool mStep;
        
        void setupPingPongFbo();
        void setupVBO();
    public:
        //void prepareSettings( Settings *settings );
        void setup();
        void resize( );
        void update();
        void draw();
        void mouseMove( MouseEvent event );
        void mouseDown( MouseEvent event );
        void mouseDrag( MouseEvent event );
        void keyDown( KeyEvent event );
        
        void setAttractorPosition(Vec3f);
    };

} // namespace gpups
