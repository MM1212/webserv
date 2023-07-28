#include "Yaml.hpp"
#include <iostream>

void YAML::RunTests() {
  const YAML::Node config = YAML::LoadFile("config/tests/yaml/1.yaml");

  std::cout << config << std::endl;
  // // Accessing scalar values
  // // Accessing sequence values
  YAML::Node hobbies = config["hobbies"];
  for (
    YAML::Node::const_iterator it = hobbies.begin<YAML::Node::Sequence>();
    it != hobbies.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& hobby = *it;
    std::cout << "- type: " << hobby["type"].as<std::string>() << std::endl;
    std::cout << "- name: " << hobby["name"].as<std::string>() << std::endl;
    std::cout << "- is a sport: " << std::boolalpha << hobby["is_sport"].as<bool>() << std::endl << std::endl;
  }
}