# CITS3002 Project

Brandon Barker

22507204

Created Using 

- OS: Manjaro Linux x86_64 
- Kernel: 5.6.15-1-MANJARO

# How to use

The makefile is location in the src directory 

Place timetables in timetables directory

Place ./startstations.sh in the src directory and run from there

There shouldn't be any issues (apart from maybe edge cases) everything works for me :)


# Searching for invalid station is slow with python
The python implementation is very slow

For a network of 20 stations (using time)

Python

	curl "localhost:4013/?to=JunctionZ"
	0.01s user 0.00s system 0% cpu 3:11.71 total

C++

	curl "localhost:4013/?to=JunctionZ"
	0.00s user 0.00s system 0% cpu 6.631 total

Yes Python is 3 mins

So it works, just the python is a bit slow :)

The total number of paths for a network with $k$ edges between $(1 \le k \le  S
-1)$ where $S$ is the number of stations is given by

$$
(S-2)(S-3)...(S-k) = \frac{(S-2)!}{(S -k - 1)!}
$$

So finding if a valid path to a node exists is quite a long process.
Interestingly the maximum number for $S \ge 3$ stations is $\approx (S - 2)!e$


