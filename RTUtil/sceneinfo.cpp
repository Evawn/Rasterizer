/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Implementation of scene information parser.  
 * 
 * Author: srm, Spring 2020
 */

#include <fstream>

#include "sceneinfo.hpp"
#include "microfacet.hpp"
#include "json.hpp"
using json = nlohmann::json;

using std::shared_ptr;
using std::make_shared;


namespace RTUtil {

// Function to convert JSON arrays easily to Eigen vectors

class json_conversion_error : public nlohmann::detail::exception {
  public:
    static json_conversion_error create(const std::string& what_arg)
    {
        std::string w = nlohmann::detail::exception::name("json_conversion_error", 502) + what_arg;
        return json_conversion_error(502, w.c_str());
    }

  private:
    json_conversion_error(int id_, const char* what_arg)
        : nlohmann::detail::exception(id_, what_arg)
    {}
};

template<class T, int n>
void from_json(const nlohmann::json &j, Eigen::Matrix<T, n, 1> &val) {
    if (!j.is_array())
        throw json_conversion_error::create("Attempt to convert non-array JSON value to a vector");
    if (j.size() != n)
        throw json_conversion_error::create("Attempt to convert a JSON array to a vector of non-matching length");
    for (int i = 0; i < n; i++)
        val[i] = j.at(i);
}

void readLightInfo(const json &sceneInfo, SceneInfo &info) {
    bool backgroundSet = false;
    for (const auto &lightInfo : sceneInfo["lights"]) {
        std::string nodeName = lightInfo["node"];
        LightType type;
        Eigen::Vector3f position, power;
        Eigen::Vector3f normal, up;
        Eigen::Vector2f size;
        Eigen::Vector3f radiance;
        float range = std::numeric_limits<float>::infinity();
        if (lightInfo["type"] == "point") {
            type = LightType::Point;
            from_json(lightInfo["position"], position);
            from_json(lightInfo["power"], power);
        } else if (lightInfo["type"] == "area") {
            type = LightType::Area;
            from_json(lightInfo["position"], position);
            from_json(lightInfo["normal"], normal);
            from_json(lightInfo["up"], up);
            from_json(lightInfo["size"], size);
            from_json(lightInfo["power"], power);
        } else if (lightInfo["type"] == "ambient") {
            type = LightType::Ambient;
            from_json(lightInfo["radiance"], radiance);
            if (lightInfo.find("range") != lightInfo.end()) {
                range = lightInfo["range"];
            }
            if (!backgroundSet) {
                info.backgroundRadiance = radiance;
                backgroundSet = true;
            }
        } else {
            std::cerr << "warning: unkonwn light type '" << lightInfo["type"] << "' encountered; ignored." << std::endl;
            continue;
        }
        // LightInfo foo(std::initializer_list<LightInfo> { nodeName, type, power, position, normal, up, size, radiance, range });
        info.lights.push_back(make_shared<LightInfo>(
            LightInfo { nodeName, type, power, position, normal, up, size, radiance, range }
            ));
    }
}

void readMaterialInfo(const json &sceneInfo, SceneInfo &info) {

    for (const auto &matlInfo : sceneInfo["materials"]) {
        Eigen::Vector3f diffuse(0.5f, 0.5f, 0.5f);
        float ior = 1.5f;
        float roughness = 0.2f;
        if (matlInfo.find("diffuse") != matlInfo.end())
            from_json(matlInfo["diffuse"], diffuse);
        if (matlInfo.find("ior") != matlInfo.end())
            ior = matlInfo["ior"];
        if (matlInfo.find("roughness") != matlInfo.end())
            roughness = matlInfo["roughness"];
        if (matlInfo.find("name") != matlInfo.end()) {
            std::string matlName = matlInfo["name"];
            info.namedMaterials.insert({matlName, make_shared<nori::Microfacet>(roughness, ior, 1.0, diffuse)});
        } else if (matlInfo.find("node") != matlInfo.end()) {
            std::string nodeName = matlInfo["node"];
            info.nodeMaterials.insert({nodeName, make_shared<nori::Microfacet>(roughness, ior, 1.0, diffuse)});
        } else {
            info.defaultMaterial = make_shared<nori::Microfacet>(roughness, ior, 1.0, diffuse);
        }
    }

}

bool readSceneInfo(const std::string infoPath, SceneInfo &info) {

    info.defaultMaterial = make_shared<nori::Microfacet>(0.1, 1.5, 1.0, Eigen::Vector3f(0.4, 0.4, 0.4));
    info.backgroundRadiance = Eigen::Vector3f::Zero();
    
    /*
     * New plan: lights specified in separate input, add them using existing nodes
     */
    json sceneInfo;
    try {
        std::ifstream infi(infoPath);
        if (infi.fail()) {
            std::cerr << "Unable to open scene information file " << infoPath << std::endl;
            return false;
        }
        infi >> sceneInfo;
    } catch (json::parse_error &e) {
        std::cerr << "Error parsing scene information file: " << e.what() << std::endl;
        return false;
    }

    readLightInfo(sceneInfo, info);

    readMaterialInfo(sceneInfo, info); 

    return true;   
}

}
