#include <concepts>
#include <exception>
#include <optional>

// Error policies for the fallible refinements factory.
namespace rvarago::refined::error {

// Models the error reporting mechanism.
template <typename Policy, typename Refinement>
concept policy = requires(Policy p, Refinement refined) {
  typename Refinement::value_type;
  typename Refinement::predicate_type;

  // On success.
  {
    p.template ok<Refinement>(refined)
  } -> std::same_as<typename Policy::template wrapper_type<Refinement>>;

  // On error.
  {
    p.template err<Refinement>()
  } -> std::same_as<typename Policy::template wrapper_type<Refinement>>;
};

// Report errors as `std::optional<Refinement>`.
struct to_optional {
  template <typename Refinement> using wrapper_type = std::optional<Refinement>;

  // Returns an engaged optional.
  template <typename Refinement>
  constexpr auto ok(Refinement refinement) const -> wrapper_type<Refinement> {
    return std::optional{std::move(refinement)};
  }

  // Returns nullopt.
  template <typename Refinement>
  constexpr auto err() const -> wrapper_type<Refinement> {
    return std::nullopt;
  }
};

// Report errors as exceptions.
struct to_exception {
  template <typename Refinement> using wrapper_type = Refinement;

  struct refinement_exception : std::exception {
    const char *what() const noexcept override {
      return "fail to refine argument due to unsastified predicate";
    }
  };

  // Returns argument unchanged.
  template <typename Refinement>
  constexpr auto ok(Refinement refinement) const -> wrapper_type<Refinement> {
    return refinement;
  }

  // Throws a `refinement_exception`.
  template <typename Refinement>
  constexpr auto err() const -> wrapper_type<Refinement> {
    throw refinement_exception{};
  }
};

} // namespace rvarago::refined::error
