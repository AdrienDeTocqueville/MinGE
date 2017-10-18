#include "Meshes/Model.h"

Model::Model():
    Mesh()
{}

Model::Model(std::string _file):
    Mesh(),
    file(_file)
{
    if (load_obj())
        loadBuffers();
}

Model::~Model()
{ }

/// Methods (protected)
bool Model::load_obj()
{
    std::string path = "Resources/" + file;
    std::ifstream wavefront((path).c_str());

    if (!wavefront)
    {
        Error::add(FILE_NOT_FOUND, "Model::load_obj() -> " + path);
        return false;
    }

    std::vector<vec3> t_vertices;
    std::vector<vec3> t_normals;
    std::vector<vec2> t_texCoords;

    std::istringstream stream;

    std::string line;
    vec3 v3;
    vec2 v2;
    vec3 min(0.0f), max(0.0f);

    while (getline(wavefront, line))
    {
        if (!line.size())
            continue;

        stream.clear();
        stream.str(line);
        stream >> line;

        if (line == "v")
        {
            stream >> v3.x >> v3.y >> v3.z;

            t_vertices.push_back(v3);

            if (v3.x > max.x)   max.x = v3.x;
            if (v3.x < min.x)   min.x = v3.x;
            if (v3.y > max.y)   max.y = v3.y;
            if (v3.y < min.y)   min.y = v3.y;
            if (v3.z > max.z)   max.z = v3.z;
            if (v3.z < min.z)   min.z = v3.z;
        }
        else if (line == "vn")
        {
            stream >> v3.x >> v3.y >> v3.z;

            t_normals.push_back(v3);
        }
        else if (line == "vt")
        {
            stream >> v2.x >> v2.y;

            t_texCoords.push_back(v2);
        }
        else if (line == "f")
        {
            unsigned vert[3], norm[3], texc[3];
            char slash;
            std::string vertex;

            for (unsigned i(0) ; i < 3 ; i++)
            {
                stream >> vertex;
                std::istringstream infos(vertex);

                infos >> vert[i] >> slash;
                vertices.push_back(t_vertices[vert[i] -1]);

                if (t_texCoords.size())
                {
                    infos >> texc[i];
                    texCoords.push_back(t_texCoords[texc[i] -1]);
                }
                else
                    texCoords.push_back(vec2(0.0f));

                if (t_normals.size())
                {
                    infos >> slash >> norm[i];
                    normals.push_back(t_normals[norm[i] -1]);
                }
                else
                    normals.push_back(vec3(0.0f));
            }
        }
        else if (line == "usemtl")
        {
            stream >> line;

            if (submeshes.size())
                submeshes.back().count = vertices.size() - submeshes.back().first;

            submeshes.push_back( Submesh(GL_TRIANGLES, vertices.size(), 0, getMaterialIndex(line)) );
        }
        else if (line == "mtllib")
        {
            stream >> line;

            load_mtl("Resources/" + line);
        }
    }

    submeshes.back().count = vertices.size() - submeshes.back().first;

//    std::cout << "--------- " << file << std::endl;
//    vec3 dim = (max-min);
//    min = (max+min)*0.5f;
//    std::cout << "Dimensions: " << dim.x << " " << dim.y << " " << dim.z << std::endl;
//    std::cout << "Centre    : " << min.x << " " << min.y << " " << min.z << std::endl;
//    std::cout << std::endl;

    return true;
}

bool Model::load_mtl(std::string _path)
{
    std::ifstream mtl(_path.c_str());

    if (!mtl)
    {
        Error::add(FILE_NOT_FOUND, "Model::load_mtl() -> " + _path);
        return false;
    }

    std::string line;
    ModelMaterial* mat = nullptr;

    while (getline(mtl, line))
    {
        if (!line.size())
            continue;

        std::istringstream stream(line);
        stream >> line;

        if (line == "#")
            continue;
        else if (line == "newmtl")
        {
            stream >> line;

            mat = new ModelMaterial(line);
            materials.push_back(mat);
        }
        else if (line == "Ka")
        {
            for (unsigned i(0) ; i < 3 ; i++)
                stream >> mat->ambient[i];
        }
        else if (line == "Kd")
        {
            for (unsigned i(0) ; i < 3 ; i++)
             stream >> mat->diffuse[i];
        }
        else if (line == "Ks")
        {
            for (unsigned i(0) ; i < 3 ; i++)
             stream >> mat->specular[i];
        }
        else if (line == "Ns")
        {
            stream >> mat->exponent;
        }
        else if (line == "map_Kd")
        {
            stream >> line;
            mat->texture = Texture::get(line);
        }
    }

    return true;
}
