# Ramsey Theory Viewer

Want to see for yourself that R(r,s) actually equals N? Look no further than this program!

This program allows you to see every nonisomorphic graph on N vertices and how it either contains a K_r or its complement contains a K_s.

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

Generate graphs and pipe into `gen-r34`, `geng 9 -q | ./gen-rMN 3 4`

Server HTML `fastapi dev server.py`

Navigate to [`http://127.0.0.1:8000/index.html`](http://127.0.0.1:8000/index.html)

### Usage

```
Usage: ./gen-rMN r s
  r: size of red clique to search for (e.g., 3)
  s: size of blue clique to search for (e.g., 4)
Reads graph6 graphs from stdin and writes results to ramsey_results.db.
  -h: show this help message
```

## More Info

More numbers and statistics to be added shortly!