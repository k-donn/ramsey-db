# Ramsey Theory Viewer

Want to see for yourself that R(r,s) actually equals N? Look no further than this program!

This program allows you to see every* nonisomorphic graph on N vertices and how it either contains a K_r or its complement contains a K_s.

## Running

### Prerequisites

#### C++

`g++`,`sqlite3`, `geng`, and `boost`.

#### Python

`sqlite3` and `fastapi`

### Compiling

`g++ -std=c++17 -O2 -o gen-rMN gen-rMN.cc -lsqlite3`

Ensure `sqlite3` is on your include path!

### Execute

Generate graphs with `./geng-util.sh`.

Make ramsey data with `./make-data.sh`.

Serve HTML `fastapi dev server.py`.

Navigate to [`http://localhost:8000/index.html`](http://localhost:8000/index.html).

### Usage

```text
Usage: ./gen-rMN r s
  r: size of red clique to search for (e.g., 3)
  s: size of blue clique to search for (e.g., 4)
Reads graph6 graphs from stdin and writes results to ramsey_results.db.
  -h: show this help message
```

## More Info

### Caveats

As seen in [A000088](https://oeis.org/A000088) the number of nonisomorphic graphs grows rapidly! Thus, it is unrealistic to generate data for all 29 quadrillion graphs on 14 vertices. The script `geng-util.sh` generates a subset of these graphs which can then be read by the program for a representative sample.

More numbers and statistics to be added shortly!
