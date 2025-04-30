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

constexpr int N = 9;

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

int main() {
	std::string line;
	std::vector<int> k3_counts;
	std::vector<std::string> k3s;
	std::vector<int> k4_counts;
	std::vector<std::string> k4s;

	while (std::getline(std::cin, line)) {
		if (line.empty() || line[0] == '>') continue;
		Graph g = graph6_to_boost(line);

		// Find K3 in g
		int k3_count = 0;
		auto k3 = find_Kk(g, 3, k3_count);

		// Complement
		Graph gc(N);
		for (int u = 0; u < N; ++u) {
			for (int v = u + 1; v < N; ++v) {
				if (!edge(u, v, g).second) {
					add_edge(u, v, gc);
				}
			}
		}
		int k4_count = 0;
		auto k4 = find_Kk(gc, 4, k4_count);

		// Serialize vectors as comma-separated strings
		std::string k3_str, k4_str;
		for (size_t i = 0; i < k3.size(); ++i) {
			k3_str += std::to_string(k3[i]);
			if (i + 1 < k3.size()) k3_str += ",";
		}
		for (size_t i = 0; i < k4.size(); ++i) {
			k4_str += std::to_string(k4[i]);
			if (i + 1 < k4.size()) k4_str += ",";
		}

		// Store results
		k3_counts.push_back(k3_count);
		k3s.push_back(k3_str);
		k4_counts.push_back(k4_count);
		k4s.push_back(k4_str);
	}

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
	const char* create_sql =
		"CREATE TABLE IF NOT EXISTS results ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"k3_count INTEGER,"
		"k3 TEXT,"
		"k4_count INTEGER,"
		"k4 TEXT"
		");";
	char* errMsg = nullptr;
	if (sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "SQL error: " << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return 1;
	}

	const char* clear_table_sql = "DELETE * FROM ramsey_results;";
	if (sqlite3_exec(db, clear_table_sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "SQL error: " << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return 1;
	}

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	// Prepare insert statement
	sqlite3_stmt* stmt;
	const char* insert_sql = "INSERT INTO results (k3_count, k3, k4_count, k4) VALUES (?, ?, ?, ?);";
	if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return 1;
	}

	// Insert data
	for (size_t i = 0; i < k3_counts.size(); ++i) {
		sqlite3_bind_int(stmt, 1, k3_counts[i]);
		sqlite3_bind_text(stmt, 2, k3s[i].c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 3, k4_counts[i]);
		sqlite3_bind_text(stmt, 4, k4s[i].c_str(), -1, SQLITE_TRANSIENT);

		if (sqlite3_step(stmt) != SQLITE_DONE) {
			std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
		}
		sqlite3_reset(stmt);
	}

	sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return 0;
}
