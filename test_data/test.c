fun climb(i, n) {
		if(i == n) return 1;
		if(i > n) return 0;
		
		return climb(i + 1, n) + climb(i + 2, n);
}

for(var i = 0; i < 10; i = i + 1) {
		print climb(0, i);
}

print "#### Test 2 ####";

var x = 0;
for(var i = 0; i < 10; i = i + 1) {
		for(var j = 0; j < 10; j = j + 1) {
				x = x + i + j;
		}
}

print x;

print "#### Test 3 ####";

var i = 0;
while (i < 10) {
		print i;
		i = i + 1;
}

print "#### Test 4 ####";

fun pretty_print(string) {
		print "pretty_print: ";
		print string;
}

pretty_print("hello");

print "#### Test 5 ####";

fun f() {
		print x;
} f();
