/*
 * Filename: asset.cpp
 *
 * Implementation of the code to interface with ASSIMP
 */

#include "asset.hpp"

#include <iostream>

Asset::Asset()
{
        scene  = NULL;
}

Asset::Asset( string fileName )
{
        scene  = NULL;
        this->openModel( fileName );
}

Asset::~Asset()
{

}

bool Asset::openModel( string fileName )
{
        // Try to load a model as given by caller at constructor
        scene = import.ReadFile( fileName,
                        aiProcess_CalcTangentSpace       |
                        aiProcess_Triangulate            |
                        aiProcess_JoinIdenticalVertices  |
                        aiProcess_SortByPType );

        // Did we have any success?
        if (!scene) {
                cerr << import.GetErrorString();
                return false;
        }

        // No error encountered so everything must have worked
        return true;
}
