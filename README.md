# OxyIR

A simple, extendable, light-weight intermediate representation. OxyIR is designed to be used with either text form, or builder pattern. 

It can be easily extended by adding custom stages in the IR compilation, though you are limited when using text form. The builder pattern is the better way to use OxyIR if you want to extend it, as you can customise every stage of the compilation process.

Currently, it is only targeting amd64, but in the future (or if you add it yourself) it can be extended to support other architectures.

## What it tries to achieve
- Simple and easy to use
- Builder API that automatically handles SSA
- Every major stage of the compilation process is extendable, so you can add your own optimisations, analyses, etc.
- C API for easy integration with other languages
- Text form IR with SSA form

## What it doesn't try to achieve
- Optimisation performance, it should be fast enough but the main focus is a simple standard system which you can extend yourself.
- A large standard library of operations (though you can add your own)

## Why use OxyIR over QBE or LLVM IR?
While QBE and LLVM IR are both powerful in their own right, OxyIR is designed to be both light-weight and extendable whilst also providing a builder API.  
QBE is great, but is missing the builder API, which makes it harder to work with in a programmatic way, whilst LLVM brings a lot of power and features but is incredibly complex, hard to work with and slow to compile. OxyIR is a middle ground, providing a simple and easy to use IR with the ability to extend it as needed, without the overhead of a large and complex system!

## Usage
Read the docs in the `docs` folder for more information on how, what and why! There are samples in the `samples` folder!

## Requirements
To compile this project you need GCC16 or later, since it uses -freflection.