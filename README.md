# cpp_refined

> Types + Predicates in C++, sort-of

A Refinement Type constrains a base type with a predicate, such that we can only produce valid instances (according to the predicate) of the base type. This is often a language-level feature that comes with a type system that can ergonomically express relationships, e.g. subtyping, flow. [LiquidHaskell](https://ucsd-progsys.github.io/liquidhaskell/) is an example

C++ does not have Refinement Types, so this library is a clumsy attempt to bring some of their benefits to the community.

## Motivation

Let's say that throughout our application we have functions that require **even** integers.

We could model this as an `int` and whenever we need it to be even we would validate it with "is_even" and make a decision based on the result. However, we risk duplicating the same validation step in different functions or, worse, we may forget to validate by mistakenly assuming it has already been validated.

Instead of just validating and immediately forgetting, we can record this validation in the types and use it as "proof" that we already validated. So we have _some_ assurance that our precondition has already been checked and therefore we don't need to check again.

Additionally, this has a nice second-order effect of pushing validation to the edges of our application. As opposed to weakening our postcondition (this function might fail due to invalid arguments checked in the implementation), we strengthen our precondition (accepting only valid arguments) and get rid of this error case altogether.

> Check out ["Parse, don't validate"](https://lexi-lambda.github.io/blog/2019/11/05/parse-don-t-validate/) for a more thorough discussion.

## Example

```cpp
using even =
      rvarago::refined::refinement<int, decltype([](auto const x) { return x % 2 == 0; })>;
      
auto do_something_else(even v) -> void {
  // deep down in the call stack do we access its base type.
  int const x = v.value;
  // act on the base type as we see fit. 
}

auto do_something(even v) -> void {
  do_something_else(v);
}

int main() {
  int const x = read_int();
  
  if (std::optional<even> e = even::make(x); e) {
    do_something(*e);
  }
  
  return 0;
}
```

With this example, we notice that not all functions need to access the underlying `int` element and operate entirely on `even`. So we validate and convert the `int` element to `even` at the very beginning and only fall back to `int` at the very last moment, when we actually need it. Both operations should ideally at the edges of our applications.

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
