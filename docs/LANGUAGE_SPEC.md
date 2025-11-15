# Oracon Language Specification

Version 0.1.0

## Overview

Oracon is a modern, dynamic programming language with optional static typing, designed for ease of use and performance.

## Design Principles

1. **Dynamic by default, static when needed**: Variables are dynamically typed but can be statically typed for performance
2. **Expressive and concise**: Clean syntax inspired by modern languages
3. **First-class functions**: Functions are values and can be passed around
4. **Object-oriented and functional**: Support both paradigms
5. **Memory safe**: Automatic memory management with garbage collection

## Basic Syntax

### Comments

```oracon
// Single-line comment

/*
   Multi-line comment
*/
```

### Variables

```oracon
// Dynamic variables
let x = 10
let name = "Oracon"
let is_active = true

// Static typing (optional)
let count: int = 42
let pi: float = 3.14159
let message: string = "Hello"

// Constants
const MAX_SIZE = 1000
```

### Data Types

#### Primitive Types
- `int` - Integer numbers (64-bit)
- `float` - Floating-point numbers (64-bit)
- `bool` - Boolean (true/false)
- `string` - UTF-8 strings
- `nil` - Null/none value

#### Compound Types
- `array` - Dynamic arrays
- `dict` - Hash maps/dictionaries
- `func` - Function type

### Operators

#### Arithmetic
```oracon
+ - * / %     // Basic arithmetic
**            // Power
```

#### Comparison
```oracon
== != < > <= >=
```

#### Logical
```oracon
and or not
&& || !       // Alternative syntax
```

#### Assignment
```oracon
= += -= *= /= %=
```

### Control Flow

#### If Statements
```oracon
if condition {
    // code
} else if another_condition {
    // code
} else {
    // code
}

// Ternary operator
let result = condition ? value1 : value2
```

#### Loops
```oracon
// While loop
while condition {
    // code
}

// For loop (range-based)
for i in 0..10 {
    // code
}

// For loop (collection)
for item in collection {
    // code
}

// For loop (with index)
for index, item in collection {
    // code
}

// Loop control
break
continue
```

#### Match Expression (Pattern Matching)
```oracon
match value {
    0 => print("zero"),
    1..10 => print("small"),
    _ => print("other")
}
```

### Functions

```oracon
// Basic function
func greet(name) {
    return "Hello, " + name
}

// Function with type annotations
func add(a: int, b: int) -> int {
    return a + b
}

// Default parameters
func greet(name = "World") {
    return "Hello, " + name
}

// Variadic functions
func sum(numbers...) {
    let total = 0
    for num in numbers {
        total += num
    }
    return total
}

// Lambda/anonymous functions
let double = func(x) { return x * 2 }
let triple = (x) => x * 3  // Arrow function syntax

// Higher-order functions
func map(array, fn) {
    let result = []
    for item in array {
        result.push(fn(item))
    }
    return result
}
```

### Arrays

```oracon
// Array creation
let numbers = [1, 2, 3, 4, 5]
let mixed = [1, "two", 3.0, true]

// Array access
let first = numbers[0]
let last = numbers[-1]

// Array slicing
let subset = numbers[1..3]  // [2, 3]

// Array methods
numbers.push(6)
numbers.pop()
numbers.length()
numbers.map(func(x) { return x * 2 })
numbers.filter(func(x) { return x > 2 })
```

### Dictionaries

```oracon
// Dictionary creation
let person = {
    "name": "Alice",
    "age": 30,
    "active": true
}

// Alternative syntax
let config = {
    timeout: 5000,
    retries: 3
}

// Access
let name = person["name"]
let age = person.age  // Dot notation

// Methods
person.keys()
person.values()
person.has("name")
```

### Classes and Objects

```oracon
class Person {
    // Constructor
    init(name, age) {
        self.name = name
        self.age = age
    }

    // Method
    greet() {
        return "Hello, I'm " + self.name
    }

    // Static method
    static create_default() {
        return Person("Unknown", 0)
    }
}

// Inheritance
class Student extends Person {
    init(name, age, grade) {
        super(name, age)
        self.grade = grade
    }

    study() {
        print(self.name + " is studying")
    }
}

// Create instance
let alice = Person("Alice", 30)
alice.greet()
```

### Modules

```oracon
// Import
import math
import io.file as file
import { sin, cos } from math

// Export
export func helper() {
    // code
}

export class MyClass {
    // class definition
}
```

### Error Handling

```oracon
// Try-catch
try {
    // code that might fail
    let result = risky_operation()
} catch error {
    print("Error: " + error.message)
} finally {
    // cleanup code
}

// Panic (fatal error)
panic("Something went wrong!")
```

### Special Features

#### String Interpolation
```oracon
let name = "World"
let message = "Hello, {name}!"  // "Hello, World!"
```

#### Spread Operator
```oracon
let arr1 = [1, 2, 3]
let arr2 = [...arr1, 4, 5]  // [1, 2, 3, 4, 5]

let dict1 = { a: 1, b: 2 }
let dict2 = { ...dict1, c: 3 }
```

#### Destructuring
```oracon
let [a, b, c] = [1, 2, 3]
let { name, age } = person
```

## Standard Library

### Core Modules
- `io` - Input/output operations
- `math` - Mathematical functions
- `string` - String manipulation
- `array` - Array utilities
- `file` - File system operations
- `net` - Network operations
- `json` - JSON parsing/serialization
- `time` - Date and time utilities

## Keywords

Reserved keywords:
```
let const func class static
if else while for in
return break continue
try catch finally throw
import export from as
true false nil
self super
match case default
and or not
```

## Future Features (Roadmap)

- Async/await for asynchronous programming
- Generators and iterators
- Decorators
- Type inference improvements
- Compile-time optimizations
- JIT compilation
