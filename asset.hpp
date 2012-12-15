/*
 * Filename: asset.hpp
 *
 * This file contains public interfaces to the asset loading code that
 * is implemented with ASSIMP (the Open Asset Import Library).
 */

#ifndef _ASSET_H
#define _ASSET_H

#ifdef __APPLE__

#include </usr/local/include/assimp/Importer.hpp>
#include </usr/local/include/assimp/scene.h>
#include </usr/local/include/assimp/postprocess.h>

#else

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#endif

class Asset {

};

#endif
