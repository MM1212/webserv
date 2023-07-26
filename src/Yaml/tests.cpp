#include "Yaml.hpp"
#include <iostream>

void YAML::RunTests() {
  const YAML::Node config = YAML::LoadFile("config/tests/1.yaml");

  std::cout << config << std::endl;
  // // Accessing scalar values
  // std::string name = config["name"].as<std::string>();
  // int age = config["age"].as<int>();

  // std::cout
  //   << "name: " << name << std::endl
  //   << "age: " << age << std::endl;
  // // Accessing map values
  // YAML::Node address = config["address"];
  // std::string street = address["street"].as<std::string>();
  // std::string city = address["city"].as<std::string>();
  // std::string state = address["state"].as<std::string>();
  // int zip = address["zip"].as<int>();
  // std::cout
  //   << "name: " << name << std::endl
  //   << "age: " << age << std::endl
  //   << "street: " << street << std::endl
  //   << "city: " << city << std::endl
  //   << "state: " << state << std::endl
  //   << "zip: " << zip << std::endl;
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