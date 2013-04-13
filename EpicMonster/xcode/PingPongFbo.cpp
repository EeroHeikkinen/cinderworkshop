//
//  SwapTexture.cpp
//  gpuPS
//
//  Created by Éric Renaud-Houde on 2013-01-06.
//
//

#include "PingPongFbo.h"
#include <assert.h>
#include <utility>

void PingPongFbo::addTexture(const Surface32f &surface)
{
    assert(mTextures.size() < mAttachments.size());
    assert(surface.getSize() == mTextureSize);
    
    gl::Texture::Format format;
    format.setInternalFormat(GL_RGBA32F_ARB);
	gl::Texture tex = gl::Texture( surface, format);
    tex.setWrap( GL_REPEAT, GL_REPEAT );
    tex.setMinFilter( GL_NEAREST );
    tex.setMagFilter( GL_NEAREST );
    mTextures.push_back( tex );
}

void PingPongFbo::reloadTextures()
{
	mFbos[mCurrentFbo].bindFramebuffer();
    gl::setMatricesWindow( getSize(), false );
    gl::setViewport( getBounds() );
    gl::clear();
    for(int i=0; i<mAttachments.size(); ++i) {
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
        
        mTextures[i].enableAndBind();
        gl::draw( mTextures[i], getBounds() );
        mTextures[i].unbind();
        mTextures[i].disable();
    }
    mFbos[mCurrentFbo].unbindFramebuffer();
	
	mFbos[!mCurrentFbo] = mFbos[mCurrentFbo];
}

void PingPongFbo::swap()
{
    mCurrentFbo = !mCurrentFbo;
}

void PingPongFbo::updateBind()
{
    mFbos[ mCurrentFbo ].bindFramebuffer();
    
    glDrawBuffers(mAttachments.size(), &mAttachments[0]);
    
    for(int i=0; i<mAttachments.size(); ++i) {
        mFbos[!mCurrentFbo].bindTexture(i, i);
    }
}

void PingPongFbo::updateUnbind()
{
    mFbos[ !mCurrentFbo ].unbindTexture();
    mFbos[ mCurrentFbo ].unbindFramebuffer();
}

void PingPongFbo::bindTexture(int textureUnit)
{
    mFbos[mCurrentFbo].bindTexture(textureUnit, textureUnit);
}

void PingPongFbo::unbindTexture()
{
    mFbos[mCurrentFbo].unbindTexture();
}

Vec2i PingPongFbo::getSize() const
{
    return mTextureSize;
}


Area PingPongFbo::getBounds() const
{
    return mFbos[0].getBounds();
}

gl::Texture PingPongFbo::getTexture(unsigned int at) {
    return mFbos[0].getTexture();
}
