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

#include <vector>

/* 2.0
#include "assimp/assimp.hpp"
#include "assimp/aiScene.h"
#include "assimp/aiPostProcess.h"
*/
//* 3.0
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
//*/

#include "cinder/Cinder.h"
#include "cinder/Color.h"
#include "cinder/TriMesh.h"
#include "cinder/Stream.h"
#include "cinder/AxisAlignedBox.h"

#include "Node.h"
#include "AssimpMesh.h"
#include "Particle.h"

namespace mndl { namespace assimp {

inline ci::Vec3f fromAssimp( const aiVector3D &v )
{
    return ci::Vec3f( v.x, v.y, v.z );
}

inline aiVector3D toAssimp( const ci::Vec3f &v )
{
    return aiVector3D( v.x, v.y, v.z );
}

inline ci::Quatf fromAssimp( const aiQuaternion &q )
{
    return ci::Quatf( q.w, q.x, q.y, q.z );
}

inline ci::Matrix44f fromAssimp( const aiMatrix4x4 &m )
{
	return ci::Matrix44f( &m.a1, true );
}

inline aiMatrix4x4 toAssimp( const ci::Matrix44f &m )
{
	return aiMatrix4x4( m.m00, m.m01, m.m02, m.m03,
						m.m10, m.m11, m.m12, m.m13,
						m.m20, m.m21, m.m22, m.m23,
						m.m30, m.m31, m.m32, m.m33 );
}

inline aiQuaternion toAssimp( const ci::Quatf &q )
{
    return aiQuaternion( q.w, q.v.x, q.v.y, q.v.z );
}

inline ci::ColorAf fromAssimp( const aiColor4D &c )
{
    return ci::ColorAf( c.r, c.g, c.b, c.a );
}

inline aiColor4D toAssimp( const ci::ColorAf &c )
{
    return aiColor4D( c.r, c.g, c.b, c.a );
}

/* 3.0
inline std::string fromAssimp( const aiString &s )
{
	return std::string( s.C_Str() );
}
*/
inline std::string fromAssimp( const aiString &s )
{
	return std::string( s.data );
}

class AssimpLoaderExc : public std::exception
{
	public:
		AssimpLoaderExc( const std::string &log ) throw()
		{
			strncpy( mMessage, log.c_str(), 512 );
		}

		virtual const char* what() const throw()
		{
			return mMessage;
		}

	private:
		char mMessage[ 513 ];
};

class AssimpNode : public mndl::Node
{
	public:
		std::vector< AssimpMeshRef > mMeshes;
};

typedef std::shared_ptr< AssimpNode > AssimpNodeRef;

class AssimpLoader
{
	public:
		AssimpLoader() {}

		//! Constructs and does the parsing of the file from \a filename.
		AssimpLoader( ci::fs::path filename );

		//! Updates model animation and skinning.
		void update();
		//! Draws all meshes in the model.
		void draw();

		//! Returns the bounding box of the static, not skinned mesh.
		ci::AxisAlignedBox3f getBoundingBox() const { return mBoundingBox; }

		//! Sets the orientation of this node via a quaternion.
		void setNodeOrientation( const std::string &name, const ci::Quatf &rot );
		//! Returns a quaternion representing the orientation of the node called \a name.
		ci::Quatf getNodeOrientation( const std::string &name );

		//! Returns the node called \a name.
		AssimpNodeRef getAssimpNode( const std::string &name );
		//! Returns the node called \a name.
		const AssimpNodeRef getAssimpNode( const std::string &name ) const;

		//! Returns the total number of meshes contained by the node called \a name.
		size_t getAssimpNodeNumMeshes( const std::string &name );
		//! Returns the \a n'th cinder::TriMesh contained by the node called \a name.
		ci::TriMesh &getAssimpNodeMesh( const std::string &name, size_t n = 0 );
		//! Returns the \a n'th cinder::TriMesh contained by the node called \a name.
		const ci::TriMesh &getAssimpNodeMesh( const std::string &name, size_t n = 0 ) const;

		//! Returns the texture of the \a n'th mesh in the node called \a name.
		ci::gl::Texture &getAssimpNodeTexture( const std::string &name, size_t n = 0 );
		//! Returns the texture of the \a n'th mesh in the node called \a name.
		const ci::gl::Texture &getAssimpNodeTexture( const std::string &name, size_t n = 0 ) const;

		//! Returns the material of the \a n'th mesh in the node called \a name.
		ci::gl::Material &getAssimpNodeMaterial( const std::string &name, size_t n = 0 );
		//! Returns the material of the \a n'th mesh in the node called \a name.
		const ci::gl::Material &getAssimpNodeMaterial( const std::string &name, size_t n = 0 ) const;

		//! Returns all node names in the model in a std::vector as std::string's.
		const std::vector< std::string > &getNodeNames() { return mNodeNames; }

		//! Enables/disables the usage of materials during draw.
		void enableMaterials( bool enable = true ) { mMaterialsEnabled = enable; }
		//! Disables the usage of materials during draw.
		void disableMaterials() { mMaterialsEnabled = false; }

		//! Enables/disables the usage of textures during draw.
		void enableTextures( bool enable = true ) { mTexturesEnabled = enable; }
		//! Disables the usage of textures during draw.
		void disableTextures() { mTexturesEnabled = false; }

		//! Enables/disables skinning, when the model's bones distort the vertices.
		void enableSkinning( bool enable = true );
		//! Disables skinning, when the model's bones distort the vertices.
		void disableSkinning() { enableSkinning( false ); }

		//! Enables/disables animation.
		void enableAnimation( bool enable = true ) { mAnimationEnabled = enable; }
		//! Disables animation.
		void disableAnimation() { mAnimationEnabled = false; }

		//! Returns the total number of meshes in the model.
		size_t getNumMeshes() const { return mModelMeshes.size(); }
		//! Returns the \a n'th mesh in the model.
		ci::TriMesh &getMesh( size_t n ) { return mModelMeshes[ n ]->mCachedTriMesh; }
		//! Returns the \a n'th mesh in the model.
		const ci::TriMesh &getMesh( size_t n ) const { return mModelMeshes[ n ]->mCachedTriMesh; }

		//! Returns the texture of the \a n'th mesh in the model.
		ci::gl::Texture &getTexture( size_t n ) { return mModelMeshes[ n ]->mTexture; }
		//! Returns the texture of the \a n'th mesh in the model.
		const ci::gl::Texture &getTexture( size_t n ) const { return mModelMeshes[ n ]->mTexture; }

		//! Returns the number of animations in the scene.
		size_t getNumAnimations() const;

		//! Sets the current animation index to \a n.
		void setAnimation( size_t n );

		//! Returns the duration of the \a n'th animation.
		double getAnimationDuration( size_t n ) const;

		//! Sets current animation time.
		void setTime( double t );

	private:
		void loadAllMeshes();
		AssimpNodeRef loadNodes( const aiNode* nd, AssimpNodeRef parentRef = AssimpNodeRef() );
		AssimpMeshRef convertAiMesh( const aiMesh *mesh );

		void calculateDimensions();
		void calculateBoundingBox( ci::Vec3f *min, ci::Vec3f *max );
		void calculateBoundingBoxForNode( const aiNode *nd, aiVector3D *min, aiVector3D *max, aiMatrix4x4 *trafo );

		void updateAnimation( size_t animationIndex, double currentTime );
		void updateSkinning();
		void updateMeshes();

		std::shared_ptr< Assimp::Importer > mImporterRef; // mScene will be destroyed along with the Importer object
		ci::fs::path mFilePath; /// model path
		const aiScene *mScene;

		ci::AxisAlignedBox3f mBoundingBox;

		AssimpNodeRef mRootNode; /// root node of scene

		std::vector< AssimpNodeRef > mMeshNodes; /// nodes with meshes
		std::vector< AssimpMeshRef > mModelMeshes; /// all meshes

		std::vector< std::string > mNodeNames;
		std::map< std::string, AssimpNodeRef > mNodeMap;

		bool mMaterialsEnabled;
		bool mTexturesEnabled;
		bool mSkinningEnabled;
		bool mAnimationEnabled;

		size_t mAnimationIndex;
		double mAnimationTime;
    
        list<Particle> mParticles;
};

} } // namespace mndl::assimp

