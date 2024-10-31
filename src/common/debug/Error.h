// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief add efu specific error handling rutins.
//===----------------------------------------------------------------------===//

#pragma once

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>

///
/// \class runtime_error_with_hint
/// \brief A custom runtime error class that includes additional information
/// about the file and line number where the error was thrown.
///
/// This class extends std::runtime_error to provide more context for runtime
/// errors by including the file name and line number.
///
/// \param message The error message.
/// \param file The name of the file where the error was thrown.
/// \param line The line number where the error was thrown.
///
/// \fn what
/// \brief Returns a C-style character string describing the error.
///
/// This function overrides the what() function from std::runtime_error to
/// include the file name and line number in the error message.
///
/// \return A C-style character string describing the error, including the file
/// name and line number.
///
class runtime_error_with_hint : public std::runtime_error {
public:
  runtime_error_with_hint(const std::string &message, const char *file,
                          int line)
      : std::runtime_error(message), message_(message), file_(file),
        line_(line) {}

  ///
  /// \brief Returns a C-style string describing the error.
  ///
  /// This function overrides the what() method from the standard exception
  /// class. It constructs a detailed error message including the original
  /// message, the file name, and the line number where the exception was
  /// thrown.
  ///
  /// \return A C-style string containing the detailed error message.
  ///
  const char *what() const noexcept override {
    std::ostringstream oss;
    oss << message_ << " (thrown at " << file_ << ":" << line_ << ")";
    what_ = oss.str();
    return what_.c_str();
  }

private:
  std::string message_;
  const char *file_;
  int line_;
  mutable std::string what_;
};

///
/// \def RETHROW_WITH_HINT
/// \brief Re-throws an exception with additional context information.
///
#define RETHROW_WITH_HINT(e)                                                   \
  throw runtime_error_with_hint((e).what(), __FILE__, __LINE__)
