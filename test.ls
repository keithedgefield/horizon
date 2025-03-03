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

    // Array add
    array3 = [];
    push(array3, 123);
    print(array3[0]);

    // Array resize
    print("before");
    for (v in array2) {
        print(v);
    }
    print("after");
    resize(array2, 2);
    for (v in array2) {
        print(v);
    }

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

    // Dictionary remove
    for (k, v in dict) {
    	print("k = " + k + ", v = " + v);
    }
    print("unset");
    unset(dict, "aaa");
    for (k, v in dict) {
    	print("k = " + k + ", v = " + v);
    }

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

    //
    // Objective-like Practice
    //

    // Class definition (this is just a dictionary, you know)
    myclass = {
        // Constructor
        new: lambda () => {
            obj = {};
	    obj.value = 0;

            // mixin() copies and injects functions of myclass to obj excluding new()
            mixin(obj, myclass);

            return obj;
        },

        // Method
        getValue: lambda (this) => {
            // Parameter "this" is same as "self" in Python.
            return this.value;
        }
    };

    // New
    obj = myclass.new();

    // Method call
    val = obj->getValue();  // equals to obj.getValue(obj);
}

func foo() {
    print("foo() is called.");
}
