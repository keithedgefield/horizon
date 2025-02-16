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
    a = "aaaaa";
    print("a = " + a + " len=" + len(a));
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

    // Dictionary literal
    a = {
        "aaa": "bbb",
        ccc: "ddd"
    };
    print(a.aaa);
    print(a["ccc"]);

    // Dictionary assignment
    dict = {};
    dict["aaa"] = "value1";
    dict["bbb"] = 123;

    // For-value loop
    for (v in array1) {
        print("v = " + v);
    }

    // For-key-value loop
    for (k, v in dict) {
        print("key = " + k + ", value = " + v);
    }

    // Lambda
    f = lambda (a) => { return a; };
    print("f(1) = " + f(1));

    // Class-like
    myclass = {
        foo: lambda () => {
	},
	bar: lambda () => {
	}
    };
}

func foo() {
    print("foo() is called.");
}
