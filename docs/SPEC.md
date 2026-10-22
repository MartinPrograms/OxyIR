# SPEC
A lot of this specification is inspired by QBE!   
The compiler is semi-linear, so definitions (functions & structs) must be defined before they are used, but struct definitions are parsed BEFORE function definitions.

Comments start with a `#` and continue to the end of the line.

Heap allocations are not supported in the IR, but you can add support by calling out to external functions that perform heap allocations.


## Typing

### Base Types
- `i1` `i8`, `i16`, `i32`, `i64`: integer types
- `f32`, `f64`: floating-point types (IEEE 754)
- `ptr`: pointer type
- `void`: void type (only valid as a return type)

Signedness is not a property of the type, but the operations performed on the type.

### Aggregate Types
Aggregate types are declared using the `struct` keyword, followed by flags, and then the name of the struct with the : prefix and then a list of types.
```
struct :structDefinition { i32, f64, ptr }
```
The above structDefinition is now a valid type that can be used in the IR. Structs can include any type, including other structs.
Structs are value types, so they are passed by value not by reference, though the variable holding the struct is always a pointer so it can be easily passed around without copying the entire struct.

#### Flags 

Structs can have the following flags: 
- `align N` aligns this struct to N bytes, where N is a power of two. By default, the struct alignment is the same as the max alignment of its member types.

### Array Types
Array types are declared using the [type, size] syntax, where type is any valid type and size is a constant expression that evaluates to an integer. The size of the array must be known at compile time.

```
[i32, 4] # an array of 4 i32 values
[:structDefinition, 2] # an array of 2 structDefinition values
```

## Data
Data is declared using the `data` keyword, followed by the type, optional flags, then the name of the data with a $ prefix and then the value. The value must be a constant expression that evaluates to a value of the given type. For structs, the value is a list of values for each field, in the order they were declared in the struct definition. For arrays, the value is a list of values for each element, in order.
When parsed this data is converted into byte arrays, and if you output the IR using the IR emitter, it will show all values in raw bytes `[0x00, 0x11]`.
With a string initializer `\0` must always be the last character!

```
data i32 $dataName  = 42
data i32 $rawBytes = [0x42, 0x42, 0x42, 0x42]
data :structDefinition $structData  = { 1, 3.14, 0 }
data [i32, 4] $arrayData = { 1, 2, 3, 4 }
data [i8, 6] thread_local mutable $stringData = "Hello\0" # Mutable thread-local data, which is an array of 6 i8 values (the string "Hello" with a null terminator)
data ([:structDefinition, 2]) -> i32 mutable $functionPointerData = 0; # A function which takes an array of 2 structDefinition values and returns an i32, which is mutable and initialized to 0 (null pointer)
data struct align 16 { i32, i32 } $inline_struct = { 10, 20 } # Declare an inline struct type, with an alignment of 16 with values 10, and 20 respectively.
```

### Flags
Data can have the following flags:
- `export`: the data is exported and can be accessed from outside the module
- `thread_local`: the data is thread-local, so each thread has its own instance of the data
- `mutable`: the data is mutable, so it can be modified (if not set, the data is immutable and cannot be modified)
- `align N`: the data is aligned to N bytes, where N is a power of 2. If not set, the data is aligned to the natural alignment of its type.

## Functions
Functions are declared using the `fn` keyword, followed by the return type, flags (optional), name (with a $ prefix), parameters and body. The body is a list of instructions, which can be either operations or control flow instructions.
For variadic functions, the last parameter must be `...` without a name or type.
```
fn i32 inline $functionName(i32 %param1, f64 %param2) {
@entry
    %result = adds i32 %param1, 1
    ret i32 %result
}

# External function: 
fn i32 extern $printf(ptr %formatString, ...) 

data [i8, 4] $stringData = "argc: %d, argv[0]: %s\0"
fn i8 export $main(i32 %argc, ptr %argv) {
@entry
    # Call the function with the parameters
    %result = call (ptr, ...) -> i32 $printf, ptr $stringData, ..., i32 %argc, ptr %argv
    ret i8 0
}
```
The symbol `$functionName` is now a function pointer to `(i32, f64) -> i32`, which is the type of the function.
Function types are declared as `(paramType1, paramType2) -> returnType` or `(paramType1, paramType2) conv -> returnType` where conv is the calling convention, as defined below. No mentioned conv is the same as `c`

To get the variadic arguments you must use vaastart on a va_list variable, and then use vaarg to retrieve each argument, which will be of the type specified in the function definition.

```
%vl = alloca va_list
vastart %vl
%arg1 = vaarg i32 %vl # retrieves the next argument from the va_list and treats it as an i32
%arg2 = vaarg f64 %vl # retrieves the next argument from the va_list and treats it as an f64
```

Function call:
A call always states the full function type of the callee, followed by the callee and then the arguments. The callee can be any value that evaluates to a pointer, so both direct calls (`$symbol`) and indirect calls (`%pointer`) use the same syntax.
``` 
%result = call (i32, f64) -> i32 $functionName, i32 42, f64 3.14
%result = call (ptr, ...) -> i32 $printf, ptr $formatString, ..., i32 42, f64 3.14 # calling a variadic function, with the variadic arguments seperated from the normal arguments with ...
%result = call (i32, i32) fast -> i32 %functionPointer, i32 1, i32 2 # indirect call through a pointer, using the fast calling convention
call (ptr) -> void $free, ptr %memory # calling a void function, so `%result =` is omitted
```

The function type of a call must follow these rules:
- For a direct call to a `$symbol`, the function type must be identical to the declared type of that function, including the calling convention.
- For an indirect call the function type cannot be checked against a declaration, so it is the only source of the calling convention, parameter types and return type. Calling a pointer with a type that does not match the function it points to is undefined behavior.
- The number of fixed arguments must equal the number of parameters in the function type, and each fixed argument must have the type of its corresponding parameter.
- If the function type is variadic, `...` must appear exactly after the last fixed argument (even if no variadic arguments follow). If the function type is not variadic, `...` must not appear.
- The result type is the return type of the function type. If the return type is `void`, `%result =` must be omitted, otherwise it must be present.

### Flags
Functions can have the following flags:
- `export`: the function is exported and can be called from outside the module
- `inline`: the function is a candidate for inlining, which means that the compiler may choose to replace calls to the function with the body of the function itself, to improve performance (this is just a hint, the compiler may choose to ignore it)
- `extern`: the function is defined outside the module

#### Conventions
- `c`: the function uses the default C calling convention
- `fast`: the function uses the fastcall calling convention
- `cold`: the function uses the coldcall calling convention
- `sysv64`: the function uses the SysV64 calling convention
- `win64`: the function uses the Win64 calling convention
- `aapcs64`: the function uses the Aapcs64 calling convention

### Labels
Labels are declared using @ followed by the name of the label. They are used as targets for control flow instructions.

```
fn void $functionWithControlFlow() {
@entry
    jnz i1 %condition, @true, @false
@true
    # do something if condition is true
    jmp @end
@false
    # do something if condition is false
    jmp @end
@end
    ret void
}
```

## Instructions
Every instruction produces a result, except for terminators and store instructions.
The signedness of an instruction is the last character of the instruction name, which is either `s` or `u`, for signed and unsigned respectively. If the instruction can be signed, or unsigned then you MUST include the signedness in the instruction name, otherwise it is invalid. For example, `add` can be either signed or unsigned, so you must use `adds` or `addu`, but `and` can only be unsigned, so you must use `and` and not `andu`.

An instruction that produces a result looks like `%result = instruction type operand1, operand2, ...`, where `%result` is the name of the variable that will hold the result of the instruction, `instruction` is the name of the instruction, `type` is the type of the result, and `operand1`, `operand2`, etc. are the operands of the instruction.

### Arithmetic Instructions
| Instruction | Operands                  | Result Type | Description                                                                        |
|-------------|---------------------------|-------------|------------------------------------------------------------------------------------|
| `add`       | `type operand1, operand2` | `type`      | Adds two values together.                                                          |
| `sub`       | `type operand1, operand2` | `type`      | Subtracts the second operand from the first operand.                               |
| `mul`       | `type operand1, operand2` | `type`      | Multiplies two values together.                                                    |
| `div`       | `type operand1, operand2` | `type`      | Divides the first operand by the second operand.                                   |
| `rem`       | `type operand1, operand2` | `type`      | Computes the remainder of the division of the first operand by the second operand. |
| `neg`       | `type operand`            | `type`      | Negates the value of the operand.                                                  |

Each of the above instructions has a signed and unsigned version. Except for `neg`, which is only valid for signed types or floating-point types.
Floating-point types do not have a signed or unsigned version and must not include the signedness in the instruction name.

### Bitwise Instructions
| Instruction | Operands                  | Result Type | Description                                                                                  |
|-------------|---------------------------|-------------|----------------------------------------------------------------------------------------------|
| `and`       | `type operand1, operand2` | `type`      | Computes the bitwise AND of two values.                                                      |
| `or`        | `type operand1, operand2` | `type`      | Computes the bitwise OR of two values.                                                       |
| `xor`       | `type operand1, operand2` | `type`      | Computes the bitwise XOR of two values.                                                      |
| `not`       | `type operand`            | `type`      | Computes the bitwise NOT of the operand.                                                     |
| `shl`       | `type operand1, operand2` | `type`      | Shifts the first operand to the left by the number of bits specified by the second operand.  |
| `shr`       | `type operand1, operand2` | `type`      | Shifts the first operand to the right by the number of bits specified by the second operand. |

The above instructions do not have signed or unsigned versions, as they operate on the bits of the values, not the values themselves. For shift instructions, the second operand must be an unsigned integer type, and the result type must be the same as the first operand.

### Comparison Instructions
| Instruction | Operands                  | Result Type | Description                                                                   |
|-------------|---------------------------|-------------|-------------------------------------------------------------------------------|
| `eq`        | `type operand1, operand2` | `i1`        | Compares two values for equality.                                             |
| `ne`        | `type operand1, operand2` | `i1`        | Compares two values for inequality.                                           |
| `lt`        | `type operand1, operand2` | `i1`        | Compares if the first operand is less than the second operand.                |
| `gt`        | `type operand1, operand2` | `i1`        | Compares if the first operand is greater than the second operand.             |
| `le`        | `type operand1, operand2` | `i1`        | Compares if the first operand is less than or equal to the second operand.    |
| `ge`        | `type operand1, operand2` | `i1`        | Compares if the first operand is greater than or equal to the second operand. |

Each of the above instructions has a signed and unsigned version. Except for `eq` and `ne`, which do not have a signed or unsigned version, and must not include the signedness in the instruction name.
Floating-point types do not have a signed or unsigned version and must not include the signedness in the instruction name.

### Memory Instructions

| Instruction    | Operands                            | Result Type | Description                                                                                                                                                                                                                                                                          |
|----------------|-------------------------------------|-------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `load`         | `type operand`                      | `type`      | Loads a value from memory. The operand must be a pointer to the type of the result.                                                                                                                                                                                                  |
| `store`        | `type destination, source`          | `void`      | Stores a value to memory. The destination must be a pointer to the type of the source, and the source must be of the same type as the destination.                                                                                                                                   |
| `alloca`       | `type [, count]`                    | `ptr`       | Allocates T * N bytes of memory on the stack, where T is the type of the result and N is the count, count can be optional and defaults to 1. Count must be a constant expression that evaluates to an integer.                                                                       |
| `alloca_align` | `type, alignment [, count]`         | `ptr`       | Allocates T * N bytes of memory on the stack, where T is the type of the result and N is the count, count can be optional and defaults to 1. Alignment must be a literal integer, and a multiple of 2. Alignment must be greater than or equal to the natural alignment of the type. |
| `gep`          | `type operand, index1, index2, ...` | `ptr`       | Computes the address of a sub-element of an aggregate data structure (struct or array). The first operand must be a pointer to the aggregate type, and the indices specify the path to the sub-element. Index integers are the same size as the system's pointer size.               |

The above instructions do not have signed or unsigned versions, as they operate on memory addresses, not values.

### Cast Instructions
| Instruction  | Operands                          | Result Type   | Description                                                                                                                                               |
|--------------|-----------------------------------|---------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `trunc`      | `result_type source_type operand` | `result_type` | Truncates a value to a smaller type. The operand must be an integer type, and the result type must be a smaller integer.                                  |
| `zext`       | `result_type source_type operand` | `result_type` | Zero-extends a value to a larger type. The operand must be an integer type, and the result type must be a larger integer type.                            |
| `sext`       | `result_type source_type operand` | `result_type` | Sign-extends a value to a larger type. The operand must be an integer type, and the result type must be a larger integer type.                            |
| `fptrunc`    | `result_type source_type operand` | `result_type` | Truncates a floating-point value to a smaller type. The operand must be a floating-point type, and the result type must be a smaller floating-point type. |
| `fpext`      | `result_type source_type operand` | `result_type` | Extends a floating-point value to a larger type. The operand must be a floating-point type, and the result type must be a larger floating-point type.     |
| `ptrtoint`   | `result_type source_type operand` | `result_type` | Converts a pointer to an integer. The operand must be a pointer type, and the result type must be an integer type.                                        |
| `inttoptr`   | `result_type source_type operand` | `result_type` | Converts an integer to a pointer. The operand must be an integer type, and the result type must be a pointer type.                                        |
| `inttofloat` | `result_type source_type operand` | `result_type` | Converts an integer to a floating point number.                                                                                                           |
| `floattoint` | `result_type source_type operand` | `result_type` | Converts a floating point number to an integer.                                                                                                           |

The following instructions have signed and unsigned version: 
- `trunc`
- `zext`
- `sext`
- `inttofloat` (if the source_type's sign should be taken into account)

But `ptrtoint` and `inttoptr` do not have signed or unsigned versions, as they operate on pointers and integers, not values.  
`floattoint` always results in a signed integer.

### Control Flow Instructions
| Instruction | Operands                     | Result Type | Description                                                                                                                                                                                                    |
|-------------|------------------------------|-------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `jmp`       | `label`                      | `void`      | Unconditional jump to the given label.                                                                                                                                                                         |
| `jnz`       | `i1 operand, @label, @label` | `void`      | Conditional jump to the first label if the operand is non-zero, otherwise jump to the second label.                                                                                                            |
| `ret`       | `type operand`               | `void`      | Returns from the current function with the given value. The operand must be of the same type as the function's return type. For void functions, the operand is omitted, the type `void` must still be present. |

### General Instructions
| Instruction | Operands                                                      | Result Type | Description                                                                                                                                                                                                                                                                                                                                                           |
|-------------|---------------------------------------------------------------|-------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `call`      | `function_type operand, type1 arg1, type2 arg2`               | `type`      | Calls a function. `function_type` is the full function type of the callee (e.g. `(i32, ptr) fast -> i32`), and the operand must be a pointer to a function of that type. The arguments must match the parameter types of the function type, and the result type is its return type. The variadic arguments, and normal arguments must be seperated with `...` as shown in the operands. If the function is `void` `%result =` needs to be omitted. See [Functions](#functions) for the full rules. |
| `vaarg`     | `result_type va_list_operand`                                 | `type`      | Retrieves the next argument from a variadic function. The operand must be a pointer to a `va_list` type, and the result type must be the type of the argument being retrieved.                                                                                                                                                                                        |
| `vastart`   | `operand`                                                     | `void`      | Initializes a `va_list` variable for use in a variadic function. The operand must be a pointer to a `va_list` type, and no type is required since the operand is always of `va_list`.                                                                                                                                                                                 | 
| `vacopy`    | `dest_va_list_operand, src_va_list_operand`                   | `void`      | Copies the state of one `va_list` variable to another. Both operands must be pointers to `va_list` types. This is useful for saving the state of a `va_list` before retrieving arguments, and restoring it later if needed.                                                                                                                                           |
| `bitcast`   | `type operand`                                                | `type`      | Casts a value from one type to another. Casts memory verbatim. Size of source & destination type must be equal.                                                                                                                                                                                                                                                       |
| `copy`      | `type operand`                                                | `type`      | Copies a value from one variable to another.                                                                                                                                                                                                                                                                                                                          |  
| `phi`       | `type, @label1 operand1, @label2 operand2, ...`               | `type`      | Selects a value based on the control flow path taken to reach the current block. The operands are pairs of values and labels, where each value is associated with a label that represents a control flow path. The result type must be the same as the type of the values. Must be the first instruction(s) in a basic block.                                         |
| `asm`       | `type output, "assembly code %0, %1", type1 arg1, type2 arg2` | `type`      | Embeds inline assembly code. The output operand specifies the type and variable that will hold the result of the assembly code. The assembly code is a string that can include placeholders for the arguments, which are specified after the assembly code. The result type must match the output operand type.                                                       |

va_list is an opaque type that is used to represent the state of a variadic function's arguments. It is only used as an operand for the `vaarg` and `vastart` instructions, and cannot be directly manipulated by any other instructions.

### Custom Instructions
Custom instructions can be added by defining a new instruction table, which requires a name, and then a list of instructions!

For example, if you wanted to add a `sqrt` instruction, you could define it in your custom instruction table like this:  
<sub>(this is not a syntax that exists in text form, but is how you'd define it in the builder pattern)</sub>
```
math_extensions {
    sqrt type, operand -> type
    add_sqrt type, operand1, operand2 -> type
}

volatile {
    load type, operand -> type
}

noresult{
    do_something void, operand
}
```

and then in your IR, you can use it like this:
```
%result = math_extensions.sqrt f64, %input
%result2 = math_extensions.add_sqrt f64, %input1, f64, %input2
%result3 = volatile.load i32, ptr, %inputPtr
```

where `math_extensions` is the name of the instruction table, and `sqrt` is the name of the instruction. The syntax for custom instructions is `instructionTable.instruction operands`, where `instructionTable` is the name of the instruction table, and `instruction` is the name of the instruction.
Operands must be specified like `type operand`, and are seperated by commas. To pass in a type you must use the syntax `type,`, this tells the parser that this value should be set to null and just pass the type.
IF the instruction does not have a result, or operands you *must* still include the type in the syntax, which would be `void`. For example:
```
noresult.do_something void, ptr %inputPtr
```
Custom instructions can not have a signed or unsigned suffix either.

This is NOT compatible with the command line interface, since that does not allow you to customise and extend the IR.