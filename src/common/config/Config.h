// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class implementing json support - configs specialized for
/// a specific detector/instrument should inherit from this class
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

#include <nlohmann/json.hpp>

#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Configurations {
class Config {
 public:
  /// \brief Default constructor (useful for unit tests)
  Config();

  /// \brief Constructor used by the EFU to load a json object from a file
  /// \param ConfigFile  Path to the json file that should be loaded
  Config(const std::string &ConfigFile);

  /// \return The file path of the json file
  std::string configFile() const {
    return mConfigFile;
  }

  /// \brief Set a new config file
  /// \param ConfigFile  Name of config file
  void setConfigFile(const std::string &ConfigFile) {
    mConfigFile = ConfigFile;
  }

  /// \brief Set a new json object
  /// \param Root  The new json object
  void setRoot(const nlohmann::json &Root) {
    mRoot = Root;
  }

  /// \return the current json object
  nlohmann::json &root() {
    return mRoot;
  }

  /// \brief Set and load the root json object from a new config file
  /// \param ConfigFile  Path for the new config file
  void setRootFromFile(const std::string &ConfigFile) {
    mConfigFile = ConfigFile;
    loadFromFile();
  }

  /// \brief Load the json object from the current config file
  void loadFromFile() {
    mRoot = Json::fromFile(mConfigFile);
  }

  /// \brief Const access to a json value associated with a given key
  /// \param Key  Access value with this key
  /// \return const reference to json value
  inline const nlohmann::json &operator[](const std::string &Key) const {
    return mRoot[Key];
  }

  /// \brief Non-const Const access to a json value associated with a given key
  /// \param Key  Access value with this key
  /// \return non-const reference to json value
  inline nlohmann::json &operator[](const std::string &Key) {
    return mRoot[Key];
  }

  /// \brief For a given key \Key, look up the value in the root json object and
  /// assign it to the \Receiver input variable
  ///
  /// \tparam T The type of the input value
  ///
  /// \param Key       The key associated with the value
  /// \param Receiver  Assign the value to this parameter
  ///
  /// \return true if the assignment was was successful, otherwise false
  template <class T>
  bool assign(const std::string &Key, T &Receiver) const {
    if (mRoot.contains(Key)) {
      try {
        Receiver = mRoot[Key].get<T>();
        if (mMask & LOG) {
          LOG(INIT, Sev::Debug, "{}: {}", Key, Receiver);
        }

        if (mMask & XTRACE) {
          const std::string message = fmt::format("{} {}", Key, Receiver);
          XTRACE(INIT, DEB, "%s", message.c_str());
        }

        return true;
      }

      // Output JSON exception information and rethrow
      catch (const nlohmann::json::exception& e) {
        fmt::print("JSON exception: {} - Exception id: {}", e.what(), e.id);
        throw(e);
      }
    } else {
      if (mMask & LOG) {
        std::string warning = fmt::format("Using default value for {}", Key);
        LOG(INIT, Sev::Info, warning);
      }

      if (mMask & CHECK) {
        const std::string error = fmt::format("JSON config - error: The requested key '{}' does not exist", Key);
        throw std::runtime_error(error);
      }
    }

    return false;
  }

protected:
  /// \brief Bits used for controlling if log and xtrace IO should be output. If the
  ///        CHECK bit is set, an error is thrown if a requested key does not exist.
  enum Flags : uint8_t {
    NONE    = 1 << 1,
    LOG     = 1 << 2, ///< Output log info
    XTRACE  = 1 << 3, ///< Output xtrace debug info
    CHECK   = 1 << 4  ///< Check and throw if json keys do not exist
  };

  /// \brief An or'ed set of bits used for controlling IO and error handling
  /// \param Mask  An or'ed set of bits
  void setMask(uint8_t Mask) {
    mMask = Mask;
  }

private:
  /// The loaded json object
  nlohmann::json mRoot;

  /// Filename of the loaded json file
  std::string mConfigFile;

  /// Mask used
  mutable uint8_t mMask;
};
} // namespace Configurations
