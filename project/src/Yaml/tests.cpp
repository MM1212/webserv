#include "Yaml.hpp"
#include <iostream>
#include <cstdlib>
#include <utils/Logger.hpp>
#include <Settings.hpp>

static void maps(const YAML::Node& config) {
  if (!config.is<YAML::Types::Map>())
    throw std::runtime_error("Expected a Map, got: " + YAML::Types::GetLabel(config.getType()));
  for (
    YAML::Node::map_const_iterator it = config.begin<YAML::Node::Map>();
    it != config.end<YAML::Node::Map>();
    ++it
    ) {
    const YAML::Node& node = it->second;
    if (!node.is<YAML::Types::Scalar>())
      throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(node.getType()));
  }
}

static void sequences(const YAML::Node& config) {
  if (!config.is<YAML::Types::Sequence>())
    throw std::runtime_error("Expected a Sequence, got: " + YAML::Types::GetLabel(config.getType()));
  for (
    YAML::Node::const_iterator it = config.begin<YAML::Node::Sequence>();
    it != config.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& node = *it;
    if (!node.is<YAML::Types::Scalar>())
      throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(node.getType()));
  }
}

static void scalarQuotes(const YAML::Node& config) {
  if (!config.is<YAML::Types::Sequence>())
    throw std::runtime_error("Expected a Sequence, got: " + YAML::Types::GetLabel(config.getType()));
  for (
    YAML::Node::const_iterator it = config.begin<YAML::Node::Sequence>();
    it != config.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& node = *it;
    if (!node.is<YAML::Types::Map>())
      throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(node.getType()));
    const YAML::Node& bookName = node["book"];
    const YAML::Node& releaseData = node["release_date"];
    if (!bookName.is<YAML::Types::Scalar>())
      throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(bookName.getType()));
    if (!releaseData.is<YAML::Types::Scalar>())
      throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(releaseData.getType()));
  }
}

static void sequencesAndMaps(const YAML::Node& config) {
  const YAML::Node hobbies = config["hobbies"];
  for (
    YAML::Node::const_iterator it = hobbies.begin<YAML::Node::Sequence>();
    it != hobbies.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& hobby = *it;
    hobby["type"].getValue();
    hobby["name"].getValue();
    hobby["is_sport"].as<bool>();
  }
}

static void flows(const YAML::Node& config) {
  if (!config.is<YAML::Types::Sequence>())
    throw std::runtime_error("Expected a Sequence, got: " + YAML::Types::GetLabel(config.getType()));
  for (
    YAML::Node::const_iterator it = config.begin<YAML::Node::Sequence>();
    it != config.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& node = *it;
    if (!node.is<YAML::Types::Map>())
      throw std::runtime_error("Expected a Map, got: " + YAML::Types::GetLabel(node.getType()));
    const YAML::Node& key = node["key"];
    const YAML::Node& value = node["value"];
    if (!key.is<YAML::Types::Scalar>())
      throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(key.getType()));
    if (!value.is<YAML::Types::Sequence>())
      throw std::runtime_error("Expected a Sequence, got: " + YAML::Types::GetLabel(value.getType()));
    for (
      YAML::Node::const_iterator it = value.begin<YAML::Node::Sequence>();
      it != value.end<YAML::Node::Sequence>();
      ++it
      ) {
      const YAML::Node& node = *it;
      if (!node.is<YAML::Types::Scalar>())
        throw std::runtime_error("Expected a Scalar, got: " + YAML::Types::GetLabel(node.getType()));
    }
  }
}

static bool test(const std::string& path, void (*handler)(const YAML::Node& node)) {
  Logger::info << "Testing " << Logger::param(path) << ".." << std::newl;

  try {
    const YAML::Node root = YAML::LoadFile(path);
    if (handler)
      handler(root);
    Logger::success << "Test " << Logger::param(path) << " passed:" << std::newl
      << Logger::param(root.expand()) << std::newl;
  }
  catch (const std::exception& e) {
    Logger::error << "Test " << Logger::param(path) << " failed: " << Logger::param(e.what()) << std::newl;
    return false;
  }
  return true;
}

void YAML::RunTests() {
  const Settings* settings = Instance::Get<Settings>();
  if (!settings->get<bool>("yaml.run_tests"))
    return;
  (void)maps;
  (void)sequences;
  (void)scalarQuotes;
  (void)sequencesAndMaps;
  (void)flows;
  int failed = 0;
  failed += !test("bin/.sys/tests/yaml/maps.yaml", &maps);
  failed += !test("bin/.sys/tests/yaml/sequences.yaml", &sequences);
  failed += !test("bin/.sys/tests/yaml/scalar_quotes.yaml", &scalarQuotes);
  failed += !test("bin/.sys/tests/yaml/sequences_map.yaml", &sequencesAndMaps);
  failed += !test("bin/.sys/tests/yaml/flows.yaml", &flows);
  failed += !test("bin/.sys/tests/wip.yaml", NULL);
  failed += !test("bin/.sys/tests/1.yaml", NULL);
  failed += !test("bin/.sys/tests/2.yaml", NULL);
  Logger::info
    << "YAML test results: " << Logger::param(failed) << " failed, "
    << Logger::param(7 - failed) << " passed!" << std::newl;
  std::exit(static_cast<bool>(failed));
}