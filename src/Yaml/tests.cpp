#include "Yaml.hpp"
#include <iostream>

static void hobbiesTest(const YAML::Node& config) {
  const YAML::Node hobbies = config["hobbies"];
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

static void test(const std::string& path, void (*handler)(const YAML::Node& node)) {
  std::cout << "Testing " << path << ".." << std::endl;
  try {
    const YAML::Node root = YAML::LoadFile(path);
    if (handler)
      handler(root);
    std::cout << "Test " << path << " passed!" << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "Test " << path << " failed: " << e.what() << std::endl;
  }
}

void YAML::RunTests() {
  (void)hobbiesTest;
  test("config/tests/yaml/1.yaml", &hobbiesTest);
  test("config/tests/example.yaml", NULL);
  test("config/tests/wip.yaml", NULL);

}