/*
 * Filename: asset.hpp
 *
 * This file contains public interfaces to the asset loading code that
 * is going to produce OpenGL scenes from Blender OBJ files.
 *
 * Much of this code comes from:
 * http://www.donkerdump.nl/node/207
 */

#ifndef _ASSET_H
#define _ASSET_H

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>

#include <string>
#include <cstring>
#include <cassert>

#include "angel/Angel.h"

class Asset3ds
{
public:
        Asset3ds(std::string filename);
        virtual void Draw() const;
        virtual void CreateVBO();
        virtual ~Asset3ds();
protected:
        void GetFaces();
        unsigned int m_TotalFaces;
        Lib3dsFile * m_model;
        GLuint m_VertexVBO, m_NormalVBO;
};

#endif
