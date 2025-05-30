#include <boost/graph/adjacency_list.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <filesystem>
#include <numeric>
#include <sqlite3.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

int N = -1;

struct RamseyResult {
	long id;
	int k_red_count, k_blue_count;
	std::string k_red, k_blue, red_edges, blue_edges;
};

const std::map<std::pair<int, int>, int> ramsey_lookup = {
	{{3, 3}, 6},
	{{3, 4}, 9},
	{{3, 5}, 14},
	{{3, 6}, 18},
	{{3, 7}, 23},
	{{3, 8}, 28},
	{{3, 9}, 36},
	{{4, 4}, 18},
	{{4, 5}, 25}
};

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

bool is_clique(const Graph& g, const std::vector<int>& verts) {
	for (size_t i = 0; i < verts.size(); ++i)
		for (size_t j = i + 1; j < verts.size(); ++j)
			if (!edge(verts[i], verts[j], g).second)
				return false;
	return true;
}

std::vector<int> find_Kk(const Graph& g, int k, int& count) {
	std::vector<int> verts(N);
	std::iota(verts.begin(), verts.end(), 0);
	std::vector<int> comb(k);
	std::vector<int> result;
	count = 0;
	std::function<bool(int, int)> search = [&](int start, int depth) {
		if (depth == k) {
			if (is_clique(g, comb)) {
				++count;
				if (result.empty())
					result = comb;
			}
			return false;
		}
		for (int i = start; i <= N - (k - depth); ++i) {
			comb[depth] = verts[i];
			search(i + 1, depth + 1);
		}
		return false;
	};
	search(0, 0);
	return result;
}

Graph graph6_to_boost(const std::string& g6) {
	Graph g(N);
	int idx = 0;
	for (int i = 1; i < (int)g6.size(); ++i) {
		unsigned char c = g6[i] - 63;
		for (int b = 5; b >= 0; --b) {
			if (idx >= N * (N - 1) / 2) break;
			if ((c >> b) & 1) {
				int v = 0, u = 0, cnt = 0;
				for (v = 1; v < N; ++v) {
					for (u = 0; u < v; ++u) {
						if (cnt == idx) goto found;
						++cnt;
					}
				}
				found:
				add_edge(u, v, g);
			}
			++idx;
		}
	}
	return g;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " r s\n"
				  << "  r: size of red clique to search for (e.g., 3)\n"
				  << "  s: size of blue clique to search for (e.g., 4)\n"
				  << "Reads graph6 graphs from stdin and writes results to ramsey_results.db.\n"
				  << "  -h: show this help message\n";
		return 1;
	}
	if (argc > 1 && std::string(argv[1]) == "-h") {
		std::cout << "Usage: " << argv[0] << " r s\n"
				  << "  r: size of red clique to search for (e.g., 3)\n"
				  << "  s: size of blue clique to search for (e.g., 4)\n"
				  << "Reads graph6 graphs from stdin and writes results to ramsey_results.db.\n"
				  << "  -h: show this help message\n";
		return 0;
	}
	auto start_time = std::chrono::high_resolution_clock::now();
	std::cout << "Processing graphs" << std::endl;
	std::string line;

	int k_red_l = atoi(argv[1]);
	int k_blue_l = atoi(argv[2]);

	auto lookup_it = ramsey_lookup.find({k_red_l, k_blue_l});
	if (lookup_it == ramsey_lookup.end()) {
		std::cerr << "No Ramsey number known for (" << k_red_l << ", " << k_blue_l << ")\n";
		return 1;
	}
	N = lookup_it->second;

	// Multithreaded reading and processing of graph data
	const size_t NUM_THREADS = std::thread::hardware_concurrency();
	std::queue<std::string> input_queue;
	std::mutex queue_mutex;
	std::condition_variable queue_cv;
	std::atomic<bool> done_reading(false);

	// Output vectors and their mutex
	std::mutex output_mutex;

	std::vector<RamseyResult> results;

	// Worker function
	auto worker = [&]() {
		while (true) {
			std::string line;
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				queue_cv.wait(lock, [&] { return !input_queue.empty() || done_reading; });
				if (input_queue.empty()) {
					if (done_reading) break;
					else continue;
				}
				line = std::move(input_queue.front());
				input_queue.pop();
			}
			if (line.empty() || line[0] == '>') continue;

			size_t comma_pos = line.find(',');
			if (comma_pos == std::string::npos) continue;
			long id = stol(line.substr(0, comma_pos));
			std::string g6_str = line.substr(comma_pos + 1);
			Graph g = graph6_to_boost(g6_str);

			// Find k_red in g
			int k_red_count = 0;
			auto k_red = find_Kk(g, k_red_l, k_red_count);

			// Complement
			Graph gc(N);
			for (int u = 0; u < N; ++u) {
				for (int v = u + 1; v < N; ++v) {
					if (!edge(u, v, g).second) {
						add_edge(u, v, gc);
					}
				}
			}
			int k_blue_count = 0;
			auto k_blue = find_Kk(gc, k_blue_l, k_blue_count);

			// Serialize vectors as comma-separated strings
			std::string k_red_str, k_blue_str;
			for (size_t i = 0; i < k_red.size(); ++i) {
				k_red_str += std::to_string(k_red[i]);
				if (i + 1 < k_red.size()) k_red_str += ",";
			}
			for (size_t i = 0; i < k_blue.size(); ++i) {
				k_blue_str += std::to_string(k_blue[i]);
				if (i + 1 < k_blue.size()) k_blue_str += ",";
			}

			std::string red_str, blue_str;
			std::vector<std::string> red_edges, blue_edges;
			for (int u = 0; u < N; ++u) {
				for (int v = u + 1; v < N; ++v) {
					if (edge(u, v, g).second) {
						red_edges.push_back(std::to_string(u) + ":" + std::to_string(v));
					}
					if (edge(u, v, gc).second) {
						blue_edges.push_back(std::to_string(u) + ":" + std::to_string(v));
					}
				}
			}
			for (size_t i = 0; i < red_edges.size(); ++i) {
				red_str += red_edges[i];
				if (i + 1 < red_edges.size()) red_str += ",";
			}
			for (size_t i = 0; i < blue_edges.size(); ++i) {
				blue_str += blue_edges[i];
				if (i + 1 < blue_edges.size()) blue_str += ",";
			}

			// Store results
			std::lock_guard<std::mutex> lock(output_mutex);
			RamseyResult result;
			result.id = id;
			result.k_red_count = k_red_count;
			result.k_red = k_red_str;
			result.k_blue_count = k_blue_count;
			result.k_blue = k_blue_str;
			result.red_edges = red_str;
			result.blue_edges = blue_str;
			results.push_back(result);
		}
	};

	// Start worker threads
	std::vector<std::thread> threads;
	for (size_t i = 0; i < NUM_THREADS; ++i) {
		threads.emplace_back(worker);
	}

	// Producer: read lines from stdin and push to queue
	while (std::getline(std::cin, line)) {
		std::unique_lock<std::mutex> lock(queue_mutex);
		input_queue.push(line);
		lock.unlock();
		queue_cv.notify_one();
	}
	done_reading = true;
	queue_cv.notify_all();

	// Join threads
	for (auto& t : threads) t.join();

	auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

	std::cout << "Generated graph data in: " << elapsed.count() << " seconds." << std::endl;

	auto db_start_time = std::chrono::high_resolution_clock::now();

	// Open SQLite database
	sqlite3* db;
	if (sqlite3_open("ramsey_results.db", &db)) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return 1;
	}

	sqlite3_exec(db, "PRAGMA synchronous = OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA temp_store = MEMORY;", nullptr, nullptr, nullptr);

	// Create table
	std::string create_sql = 
		"CREATE TABLE IF NOT EXISTS r" + std::string(argv[1]) + std::string(argv[2]) + " ("
		"id INTEGER PRIMARY KEY,"
		"k_red_count INTEGER,"
		"k_red TEXT,"
		"k_blue_count INTEGER,"
		"k_blue TEXT,"
		"red_edges TEXT,"
		"blue_edges TEXT"
		");";
	char* errMsg = nullptr;
	if (sqlite3_exec(db, create_sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "SQL error: " << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return 1;
	}

	// Build table name from argv[1] and argv[2]
	std::string table_name = "r" + std::string(argv[1]) + std::string(argv[2]);

	std::string clear_table_sql = "DELETE FROM " + table_name + ";";
	if (sqlite3_exec(db, clear_table_sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "SQL error: " << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return 1;
	}

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	// Prepare insert statement
	sqlite3_stmt* stmt;
	std::string insert_sql = "INSERT INTO " + table_name + " (id, k_red_count, k_red, k_blue_count, k_blue, red_edges, blue_edges) VALUES (?, ?, ?, ?, ?, ?, ?);";
	if (sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return 1;
	}

	// Insert data
	for (size_t i = 0; i < results.size(); ++i) {
		sqlite3_bind_int(stmt, 1, results[i].id);
		sqlite3_bind_int(stmt, 2, results[i].k_red_count);
		sqlite3_bind_text(stmt, 3, results[i].k_red.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 4, results[i].k_blue_count);
		sqlite3_bind_text(stmt, 5, results[i].k_blue.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 6, results[i].red_edges.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 7, results[i].blue_edges.c_str(), -1, SQLITE_TRANSIENT);


		if (sqlite3_step(stmt) != SQLITE_DONE) {
			std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
		}
		sqlite3_reset(stmt);
	}

	sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	auto db_end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> db_elapsed = db_end_time - db_start_time;
	std::cout << "Wrote to db in: " << db_elapsed.count() << " seconds." << std::endl;
	
	std::chrono::duration<double> full_elapsed = db_end_time - start_time;
	std::cout << "Total time: " << full_elapsed.count() << " seconds." << std::endl;
	return 0;
}
