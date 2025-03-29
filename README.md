# cpp_refined

> Types + Predicates in C++, sort-of

A Refinement Type constrains a ground type with a predicate, such that we can only produce valid instances (according to the predicate) of the ground type. This is often a language-level feature that comes with a type system that can ergonomically express relationships, e.g. subtyping, flow. [LiquidHaskell](https://ucsd-progsys.github.io/liquidhaskell/) is an example

C++ does not have Refinement Types, so this library is a clumsy attempt to bring some of their benefits to the community.

## Motivation

Let's say that throughout our application we have functions that require integers that are **even** and **less than 10**.

We could model this as an `int` and whenever we need it to be even we would validate it with "is_even" and make a decision based on the result. However, we risk duplicating the same validation step in different functions or, worse, we may forget to validate by mistakenly assuming it has already been validated.

Instead of just validating and immediately forgetting, we can record this validation in the types and use it as "proof" that we already validated. So we have _some_ assurance that our precondition has already been checked and therefore we don't need to check again.

Additionally, this has a nice second-order effect of pushing validation to the edges of our application. As opposed to weakening our postcondition (this function might fail due to invalid arguments checked in the implementation), we strengthen our precondition (accepting only valid arguments) and get rid of this error case altogether.

> Check out ["Parse, don't validate"](https://lexi-lambda.github.io/blog/2019/11/05/parse-don-t-validate/) for a more thorough discussion.

## Example

```cpp
// a refinement for ints constrained to be even.
using even =
      rvarago::refined::refinement<int, [](auto const x) { return x % 2 == 0; }>;

// refinements can be further refined, e.g. all x such that x is even and x < 10.
using even_lt_10 =
      rvarago::refined::refinement<int, [](auto const x) { return x < 10; }, even>;

auto do_something_else(even v) -> void {
  // deep down in the call stack do we access its ground type.
  int const x = v.value;
  // act on the ground type as we see fit. 
}

auto do_something(even_lt_10 v) -> void {
  do_something_else(v);
}

int main() {
  int const x = read_int();
  
  // the default error policy gives an std::optional back.
  if (std::optional<even_lt_10> e = even_lt_10::make(x); e) {
    do_something(*e);
  }
  
  return 0;
}
```

With this example, we notice that not all functions need access to the underlying `int` element and operate entirely on `even_lt_10` or its "super-type" `even`. So we validate and convert the `int` element into `even_lt_10` at the very beginning and only fall back to `int` at the very last moment, when we actually need it. Both operations should ideally at the edges of our applications.

Although we reported errors via `std::optional` in the example, we can customise it, e.g. to throw an exception with the built-in `even::make<refined::error::to_exception>` or define a whole user-provided policy.

## Requirements

C++20

## Usage

This is a header-only library. See [`refined.hpp`](include/rvarago/refined.hpp).

- (Optional) Link against the INTERFACE `rvarago::refined` target in your CMake build.
- Include `rvarago/refined.hpp`

## Contributing

This repository has a [`flake.nix`](./flake.nix) with everything I need for development/CI (toolchain, language server, etc).

Furthermore, with:

```sh
nix develop -c check
```

You run all CI checks locally.
