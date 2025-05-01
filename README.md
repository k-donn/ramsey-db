# Ramsey Theory Viewer

Want to see for yourself that R(3,4) actually equals 9? Look no further than this program!

This program allows you to see every nonisomorphic graph on 9 vertices and how it either contains a K_3 or its complement contains a K_4.

## Running

### Prerequisites

#### C++

`g++`,`sqlite3`, `geng`, and `boost`.

#### Python

`sqlite3` and `fastapi`

### Compiling

`g++ -std=c++17 -O2 -o gen-r34 gen-r34.cc -lsqlite3`

Ensure `sqlite3` is on your include path!

### Execute

Generate graphs and pipe into `gen-r34`, `geng 9 -q | ./gen-r34`

Server HTML `fastapi dev server.py`

Navigate to [`http://127.0.0.1:8000/index.html`](http://127.0.0.1:8000/index.html)

## More Info

More numbers and statistics to be added shortly!