//
// This is a syntax example.
//

func main() {
    // Assignment
    a = 0;
    b = 0.1;
    c = "string";

    // Function call
    print("Hello, World!");
    foo();

    // String
    print("a = " + a);
    print("b = " + b);
    print("c = " + c);

    // Array literal
    array1 = [0, 1, 2];

    // Array assignment
    array2 = [];
    array2[0] = 3;
    array2[1] = 4;
    array2[2] = 5;

    // Array reference
    print("array1[0] = " + array1[0]);

    // For-value loop
    for (v in array1) {
        print("v = " + v);
    }

    // Dictionary
    dict = {};
    dict["aaa"] = "value1";
    dict["bbb"] = 123;

    // For-key-value loop
    for (k, v in dict) {
        print("key = " + k + ", value = " + v);
    }
}

func foo() {
    print("foo() is called.");
}
