#!/usr/bin/awk -f

function random(n) {
	"exec od -An </dev/urandom" | getline num
	return num % n
}

BEGIN {
	data = "exec tr -cd -- '-a-zA-Z0-9\n' </dev/urandom"

	first = 1
	while (data | getline) {
		if (random(2) && !first) {
			print(" " $0)
			continue
		}
		first = 0

		col = random(26) + 1
		out = substr($0, 1, col)
		$0 = substr($0, col + 1)
		n = random(30)
		for (i = 0; i <= n; i++) {
			col = random(30) + 5
			if (length($0) < col)
				continue
			eq = random(int(col / 2)) + 1
			out = out substr($0, 1, eq) "=" substr($1, eq + 1, col) ";"
			$0 = substr($0, col + 1)
		}
		out = out $0 ":"
		data | getline
		out = out $0
		if (out ~ "\n" || out !~ ":")
			exit(1)
		print(out)
	}

	close(cmd)
}
