/*
 * Filename: asset.cpp
 *
 * Implementation of the code to interface with ASSIMP
 */

#include "asset.hpp"

// Load 3DS model
Asset3ds::Asset3ds(std::string filename)
{
        m_VertexVBO = 0;
        m_NormalVBO = 0;
        m_TotalFaces = 0;

        m_model = lib3ds_file_load(filename.c_str());
        // If loading the model failed, we throw an exception
        if (!m_model) {
                throw strcat("Unable to load ", filename.c_str());
        }
}

// Destructor
Asset3ds::~Asset3ds()
{
        glDeleteBuffers(1, &m_VertexVBO);
        glDeleteBuffers(1, &m_NormalVBO);

        if (m_model != NULL) {
                lib3ds_file_free(m_model);
        }
}

// Copy vertices and normals to the memory of the GPU
void Asset3ds::CreateVBO()
{
        assert(m_model != NULL);

        // Calculate the number of faces we have in total
        GetFaces();

        // Allocate memory for our vertices and normals
        Lib3dsVector * vertices = new Lib3dsVector[m_TotalFaces * 3];
        Lib3dsVector * normals = new Lib3dsVector[m_TotalFaces * 3];

        Lib3dsMesh * mesh;
        unsigned int FinishedFaces = 0;
        // Loop through all the meshes
        for (mesh = m_model->meshes; mesh != NULL; mesh = mesh->next) {
                lib3ds_mesh_calculate_normals(mesh,
                                &normals[FinishedFaces * 3]);
                // Loop through every face
                for (unsigned int cur_face = 0; cur_face < mesh->faces;
                                cur_face++) {
                        Lib3dsFace * face = &mesh->faceL[cur_face];
                        for (unsigned int i = 0; i < 3; i++) {
                                memcpy(&vertices[FinishedFaces * 3 + i],
                                                mesh->pointL[face->points[i]].pos,
                                                sizeof(Lib3dsVector));
                        }
                        FinishedFaces++;
                }
        }

        // Generate a Vertex Buffer Object and store it with our vertices
        glGenBuffers(1, &m_VertexVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Lib3dsVector) * 3 * m_TotalFaces,
                        vertices, GL_STATIC_DRAW);

        // Generate another Vertex Buffer Object and store the normals in it
        glGenBuffers(1, &m_NormalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_NormalVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Lib3dsVector) * 3 * m_TotalFaces,
                        normals, GL_STATIC_DRAW);

        // Clean up our allocated memory
        delete vertices;
        delete normals;

        // We no longer need lib3ds
        lib3ds_file_free(m_model);
        m_model = NULL;
}

// Count the total number of faces this model has
void Asset3ds::GetFaces()
{
        assert(m_model != NULL);

        m_TotalFaces = 0;
        Lib3dsMesh * mesh;
        // Loop through every mesh
        for (mesh = m_model->meshes; mesh != NULL; mesh = mesh->next) {
                // Add the number of faces this mesh has to the total faces
                m_TotalFaces += mesh->faces;
        }
}

// Render the model using Vertex Buffer Objects
void Asset3ds::Draw() const
{
        assert(m_TotalFaces != 0);

        // Enable vertex and normal arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        // Bind the vbo with the normals
        glBindBuffer(GL_ARRAY_BUFFER, m_NormalVBO);
        // The pointer for the normals is NULL which means that OpenGL will use the currently bound vbo
        glNormalPointer(GL_FLOAT, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
        glVertexPointer(3, GL_FLOAT, 0, NULL);

        // Render the triangles
        glDrawArrays(GL_TRIANGLES, 0, m_TotalFaces * 3);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
}
