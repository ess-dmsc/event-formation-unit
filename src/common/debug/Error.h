
#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <string>
#include <sstream>

class error_with_hint : public std::exception {
public:
    error_with_hint(const std::string& message, const char* file, int line)
        : message_(message), file_(file), line_(line) {}

    const char* what() const noexcept override {
        std::ostringstream oss;
        oss << message_ << " (thrown at " << file_ << ":" << line_ << ")";
        what_ = oss.str();
        return what_.c_str();
    }

private:
    std::string message_;
    const char* file_;
    int line_;
    mutable std::string what_;
};

#define RETHROW_WITH_HINT(e) throw error_with_hint((e).what(), __FILE__, __LINE__)

#endif // ERROR_H