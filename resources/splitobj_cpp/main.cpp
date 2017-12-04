#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <iomanip>

struct vertex {
    vertex(double x,double y,double z):x(x),y(y),z(z) {}
    double x,y,z;
};

struct face {
    face(int i1,int i2,int i3):i1(i1),i2(i2),i3(i3) {}
    int i1,i2,i3;
};

int main (int argc, const char * argv[]) {
    if(argc !=3)
        std::cout << "Usage: objsplitter input.obj outputdir" << std::endl;
    auto input_obj = argv[1];
    auto output_dir = argv[2];
    
    auto infile = std::ifstream(input_obj);
    std::cout << "Opening " << argv[1] << std::endl;
    std::string line;
    std::vector<vertex> vertices;
    std::string mtllib;
    std::getline(infile, mtllib);
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        std::string line_type;
        iss >> line_type;
        if(line_type == "v") {
            double x,y,z;
            iss >> x >> y >> z;
            vertices.push_back(vertex(x,y,z));
        }
        else if(line_type == "o") {
            std::string uuid;
            iss >> uuid;
            std::string material;
            std::getline(infile, material);
            std::vector<face> faces;
            std::set<int> indices;
            while (infile.peek()=='f'){
                std::string line_f;
                std::getline(infile, line_f);
                std::istringstream iss_f(line_f);
                int i1, i2, i3;
                char c;
                iss_f >> c >> i1 >> i2 >> i3;
                faces.push_back(face(i1,i2,i3));
                indices.insert(i1);
                indices.insert(i2);
                indices.insert(i3);
            }
            
            std::ofstream ofs;
            ofs.open(output_dir + uuid + ".obj");
            ofs << mtllib << std::endl;
            ofs << std::fixed << std::setprecision(2);
            int i = 0;
            std::map<int,int> index_remap;
            for(auto index : indices) {
                auto v = vertices[index-1];
                ofs << "v " << v.x << " " << v.y << " " << v.z <<std::endl;
                index_remap.insert(std::pair<int,int>(index, ++i));
            }
            ofs << "o " << uuid << std::endl;
            ofs << material << std::endl;
            for (auto face : faces) {
                ofs << "f " << index_remap[face.i1] << " " << index_remap[face.i2] << " " << index_remap[face.i3] << std::endl;
            }
            ofs.close();
        }
    }
    infile.close();
    return 0;
}
