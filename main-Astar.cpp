#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <climits> // Header untuk INT_MAX

// Ukuran grid
const int GRID_SIZE = 20;
const int CELL_SIZE = 30;

// Fungsi heuristik (menggunakan jarak Manhattan)
int heuristic(const sf::Vector2i& a, const sf::Vector2i& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

// Fungsi untuk memeriksa apakah sel valid untuk dilalui
bool isValidCell(int x, int y, const std::vector<std::vector<int>>& grid) {
    return (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE && grid[x][y] == 0);
}

// Comparator untuk priority_queue
struct CompareNodes {
    bool operator()(const std::pair<int, sf::Vector2i>& a, const std::pair<int, sf::Vector2i>& b) const {
        return a.first > b.first; // Mengurutkan berdasarkan fCost (biaya total)
    }
};

// Algoritma A* untuk pathfinding
std::vector<sf::Vector2i> aStarPath(const sf::Vector2i& start, const sf::Vector2i& end, std::vector<std::vector<int>>& grid) {
    std::vector<std::vector<bool>> visited(GRID_SIZE, std::vector<bool>(GRID_SIZE, false));
    std::vector<std::vector<sf::Vector2i>> parent(GRID_SIZE, std::vector<sf::Vector2i>(GRID_SIZE, {-1, -1}));
    std::vector<std::vector<int>> gCost(GRID_SIZE, std::vector<int>(GRID_SIZE, INT_MAX));
    std::priority_queue<std::pair<int, sf::Vector2i>, std::vector<std::pair<int, sf::Vector2i>>, CompareNodes> openSet;

    gCost[start.x][start.y] = 0;
    openSet.emplace(heuristic(start, end), start);

    std::vector<sf::Vector2i> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!openSet.empty()) {
        sf::Vector2i current = openSet.top().second;
        openSet.pop();

        if (current == end) {
            // Rekonstruksi jalur dari end ke start
            std::vector<sf::Vector2i> path;
            for (sf::Vector2i at = end; at != sf::Vector2i(-1, -1); at = parent[at.x][at.y]) {
                path.push_back(at);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        if (visited[current.x][current.y]) continue;
        visited[current.x][current.y] = true;

        for (const auto& dir : directions) {
            int newX = current.x + dir.x;
            int newY = current.y + dir.y;

            if (isValidCell(newX, newY, grid) && !visited[newX][newY]) {
                int tentativeG = gCost[current.x][current.y] + 1; // Cost antar node di grid dianggap 1

                if (tentativeG < gCost[newX][newY]) {
                    gCost[newX][newY] = tentativeG;
                    int fCost = tentativeG + heuristic({newX, newY}, end);
                    parent[newX][newY] = current;
                    openSet.emplace(fCost, sf::Vector2i(newX, newY));
                }
            }
        }
    }

    return {}; // Kembalikan path kosong jika tidak ada jalur
}

int main() {
    sf::RenderWindow window(sf::VideoMode(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE), "Path Planning Game - A* Algorithm");
    window.setFramerateLimit(60);

    std::vector<std::vector<int>> grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));
    sf::Vector2i start(0, 0);
    sf::Vector2i end(GRID_SIZE - 1, GRID_SIZE - 1);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed) {
                int x = event.mouseButton.x / CELL_SIZE;
                int y = event.mouseButton.y / CELL_SIZE;

                if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE && grid[x][y] == 0) {
                    grid[x][y] = 1; // Tandai sebagai obstacle
                }
            }
        }

        // Gambar grid
        window.clear(sf::Color::White);
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(i * CELL_SIZE, j * CELL_SIZE);
                cell.setOutlineColor(sf::Color::Black);
                cell.setOutlineThickness(1);

                if (grid[i][j] == 1) {
                    cell.setFillColor(sf::Color::Black); // Obstacle
                } else if (sf::Vector2i(i, j) == start) {
                    cell.setFillColor(sf::Color::Green); // Start
                } else if (sf::Vector2i(i, j) == end) {
                    cell.setFillColor(sf::Color::Red); // End
                }

                window.draw(cell);
            }
        }

        // Hitung jalur
        std::vector<sf::Vector2i> path = aStarPath(start, end, grid);
        for (const auto& p : path) {
            sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            pathCell.setPosition(p.x * CELL_SIZE, p.y * CELL_SIZE);
            pathCell.setFillColor(sf::Color::Cyan);
            window.draw(pathCell);
        }

        window.display();
    }

    return 0;
}
