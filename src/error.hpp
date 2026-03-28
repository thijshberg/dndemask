#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
namespace dndemask::util {
struct [[nodiscard]] Error {
  Error() {};

  Error(std::string e) : m_message(e) {};
  template <typename... Ts> Error(Ts... errors) {
    std::stringstream ss;
    ((ss << ' ' << errors), ...);
    m_message = ss.str();
  }
  const std::string message() const { return m_message; }
  const void print() const { std::cerr << m_message << '\n'; }
  void append(const std::string &value) { m_message += value; }

private:
  std::string m_message = "Generic error";
};
template <typename T> struct [[nodiscard]] ErrorOr {
  ErrorOr() : m_err(false) {}
  ErrorOr(Error e) : m_err(true), m_error(e) {};
  ErrorOr(T &&value) : m_err(false), m_value(value) {};
  ErrorOr(const T &value) : m_err(false), m_value(value) {};
  const bool is_error() const { return m_err; }
  const bool is_value() const { return not m_err; }
  Error &&error() { return std::move(*m_error); }
  T &&value() { return std::move(*m_value); };
  T &&operator*() { return std::move(*m_value); };

private:
  bool m_err;
  std::optional<T> m_value{};
  std::optional<Error> m_error{};
};
template <> struct [[nodiscard]] ErrorOr<void> {
  ErrorOr() : m_err(false) {}
  ErrorOr(Error e) : m_err(true), m_error(e) {};
  const bool is_error() const { return m_err; }
  const bool is_value() const { return not m_err; }
  void value() {};
  const Error error() const { return *m_error; }

private:
  bool m_err;
  std::optional<Error> m_error{};
};

template <typename T>
ErrorOr<std::vector<T>> flatten_error(std::vector<ErrorOr<T>> &&input) {
  std::vector<T> res;
  res.reserve(input.size());
  for (auto &item : input) {
    if (item.is_value()) {
      res.push_back(item.value());
    } else {
      return item.error();
    }
  }
  return res;
}

} // namespace dndemask::util
#define TRY(statement)                                                         \
  ({                                                                           \
    auto _tmp_value = (statement);                                             \
    if (_tmp_value.is_error()) {                                               \
      return _tmp_value.error();                                               \
    }                                                                          \
    _tmp_value.value();                                                        \
  })
#define MUST(statement)                                                        \
  do {                                                                         \
    auto _tmp_value = (statement);                                             \
    if (_tmp_value.is_error()) {                                               \
      throw std::logic_error{std::string{"Failed requirement: "} +             \
                             #statement};                                      \
    };                                                                         \
  } while (0);
