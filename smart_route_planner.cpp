#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

// Structure to hold the result of a routing algorithm
struct RouteResult {
    vector<int> path;
    int cost;
};

/*
 * Time Complexity Explanation:
 * 
 * 1. Greedy Algorithm:
 *    - Time Complexity: O(N^2), where N is the number of cities.
 *      At each of the N cities, we search through up to N cities to find the nearest unvisited city. 
 *      This is extremely fast but does not guarantee the globally optimal route.
 * 
 * 2. Dynamic Programming (TSP using Bitmask):
 *    - Time Complexity: O(N^2 * 2^N), where N is the number of cities.
 *      There are 2^N possible subsets of visited cities (represented by the bitmask), and for each subset, 
 *      the last visited city can be any of the N cities. Evaluating the next city takes O(N) time.
 *      This guarantees the optimal route but becomes computationally expensive for N > 20. 
 *      (For this program, N is constrained to a maximum of 10, making it run instantaneously).
 */

// Function prototypes
RouteResult greedyRoute(const vector<vector<int>>& dist);
RouteResult tspDP(const vector<vector<int>>& dist);
int tspHelper(const vector<vector<int>>& dist, int mask, int pos, vector<vector<int>>& dp, vector<vector<int>>& parent);

int main() {
    int choice;
    cout << "Smart Route Planner (Greedy vs Dynamic Programming)\n";
    cout << "---------------------------------------------------\n";
    cout << "1. Use predefined sample dataset\n";
    cout << "2. Enter custom dataset\n";
    cout << "Enter choice: ";
    cin >> choice;

    vector<vector<int>> dist;

    if (choice == 1) {
        // Sample dataset specifically provided for validation and comparison.
        dist = {
            { 0, 10, 15, 20},
            {10,  0, 35, 25},
            {15, 35,  0, 30},
            {20, 25, 30,  0}
        };
        cout << "\nUsing sample dataset with 4 cities.\n";
    } else {
        int n;
        cout << "\nEnter number of cities (max 10): ";
        cin >> n;
        
        if (n < 2 || n > 10) {
            cout << "Invalid number of cities. Must be between 2 and 10.\n";
            return 1;
        }
        
        // Matrix is explicitly constrained to be square (n x n) by defining the vectors symmetrically
        dist.assign(n, vector<int>(n, 0));
        cout << "Enter the adjacency matrix (" << n << "x" << n << ") representing distances:\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cin >> dist[i][j];
                if (dist[i][j] < 0) {
                    cout << "Error: Negative distances are physically invalid for this context.\n";
                    return 1;
                }
            }
        }
    }

    int numCities = dist.size();
    
    // City Mapping Output
    cout << "\n--- Dataset Details ---\n";
    cout << "City Mapping:\n";
    for (int i = 0; i < numCities; ++i) {
        cout << i << " -> " << char('A' + i) << "\n";
    }

    // Distance Matrix Output
    cout << "\nDistance Matrix:\n";
    cout << setw(5) << " ";
    for (int i = 0; i < numCities; ++i) {
        cout << setw(5) << char('A' + i);
    }
    cout << "\n";
    
    for (int i = 0; i < numCities; ++i) {
        cout << setw(5) << char('A' + i);
        for (int j = 0; j < numCities; ++j) {
            cout << setw(5) << dist[i][j];
        }
        cout << "\n";
    }

    cout << "\n--- Results ---\n\n";

    // --- GREEDY APPROACH ---
    RouteResult greedy = greedyRoute(dist);
    cout << "Greedy Route (cycle): ";
    for (size_t i = 0; i < greedy.path.size(); ++i) {
        cout << char('A' + greedy.path[i]);
        if (i < greedy.path.size() - 1) {
            cout << " -> ";
        }
    }
    cout << "\nCost: " << greedy.cost << "\n\n";

    // --- DYNAMIC PROGRAMMING APPROACH ---
    RouteResult optimal = tspDP(dist);
    cout << "==============================\n";
    cout << "OPTIMAL ROUTE SUMMARY\n";
    cout << "==============================\n";
    cout << "Route: ";
    for (size_t i = 0; i < optimal.path.size(); ++i) {
        cout << char('A' + optimal.path[i]);
        if (i < optimal.path.size() - 1) {
            cout << " -> ";
        }
    }
    cout << "\nCost Breakdown: ";
    for (size_t i = 0; i < optimal.path.size() - 1; ++i) {
        int u = optimal.path[i];
        int v = optimal.path[i+1];
        cout << char('A' + u) << "->" << char('A' + v) << "(" << dist[u][v] << ")";
        if (i < optimal.path.size() - 2) {
            cout << " + ";
        }
    }
    cout << " = " << optimal.cost << "\n";
    cout << "Total Cost: " << optimal.cost << "\n\n";

    // --- COMPARISON ---
    int diff = greedy.cost - optimal.cost;
    int absDiff = (diff > 0) ? diff : -diff;
    cout << "Absolute Difference: " << absDiff << "\n";
    
    // Handling division by zero in edge cases
    if (greedy.cost > 0) {
        double percentage = ((double)diff / greedy.cost) * 100.0;
        if (diff > 0) {
            cout << "Improvement: " << fixed << setprecision(1) << percentage << "%\n";
        } else if (diff == 0) {
            cout << "Improvement: 0.0% (Greedy found the optimal route).\n";
        } else {
            cout << "Improvement: " << fixed << setprecision(1) << percentage 
                 << "% (Greedy paradoxically found a better route, implying potentially invalid distances)\n";
        }
    } else {
        cout << "Improvement: N/A\n";
    }

    return 0;
}

// Implement nearest-neighbor greedy algorithm
RouteResult greedyRoute(const vector<vector<int>>& dist) {
    int n = dist.size();
    vector<bool> visited(n, false);
    vector<int> path;
    int cost = 0;
    
    int current = 0; // Always start from city 0
    visited[current] = true;
    path.push_back(current);
    
    cout << "[Greedy Execution Steps]\n";
    
    // Visit the remaining N-1 cities
    for (int step = 0; step < n - 1; ++step) {
        int nextCity = -1;
        int minDistance = 1e9;
        
        for (int i = 0; i < n; ++i) {
            // Find the nearest unvisited city from the current city
            if (!visited[i] && dist[current][i] < minDistance) {
                minDistance = dist[current][i];
                nextCity = i;
            }
        }
        
        cout << "  Step " << step + 1 << ": From " << char('A' + current) << " -> " << char('A' + nextCity) 
             << " (cost = " << minDistance << "), Running Total = " << cost + minDistance << "\n";
             
        visited[nextCity] = true;
        path.push_back(nextCity);
        cost += minDistance;
        current = nextCity;
    }
    
    // Complete the cycle: return to the starting city (0) for a fair comparison against TSP DP.
    cout << "  Step " << n << ": Return from " << char('A' + current) << " -> A (cost = " << dist[current][0] 
         << "), Running Total = " << cost + dist[current][0] << "\n\n";
         
    cost += dist[current][0];
    path.push_back(0);
    
    return {path, cost};
}

// Initiate Dynamic Programming approach with Bitmasking
RouteResult tspDP(const vector<vector<int>>& dist) {
    int n = dist.size();
    
    // DP table: rows represent the visited bitmask, columns represent the current city
    // dp[mask][pos] stores the minimum cost to complete the tour from 'pos' given visited set 'mask'
    // Initialized to -1 to signify an uncomputed state.
    vector<vector<int>> dp(1 << n, vector<int>(n, -1));
    
    // Parent table to keep track of the chosen path to effectively reconstruct the optimal route
    vector<vector<int>> parent(1 << n, vector<int>(n, -1));
    
    // 0th city is the starting point. Mask = 1 (meaning the 0th bit is set, city 0 is visited)
    int minCost = tspHelper(dist, 1, 0, dp, parent);
    
    vector<int> path;
    int currMask = 1;
    int currPos = 0;
    
    path.push_back(0); // Start at 0
    
    cout << "======================================\n";
    cout << "DP EXECUTION (STEP-BY-STEP)\n";
    cout << "======================================\n";
    cout << "(Mask bits indicate visited cities. Only final optimal decisions shown.)\n\n";
    
    int step = 1;
    int runningCost = 0;

    // Reconstruct the optimal path using the parent table
    while (currMask != ((1 << n) - 1)) {
        int nextCity = parent[currMask][currPos];
        int stepCost = dist[currPos][nextCity];
        runningCost += stepCost;
        
        cout << "--------------------------------------\n";
        cout << "Step " << step++ << "\n";
        cout << "> Current City: " << char('A' + currPos) << "\n";
        
        cout << "  Visited: {";
        bool first = true;
        for (int j = 0; j < n; ++j) {
            if (currMask & (1 << j)) {
                if (!first) cout << ", ";
                cout << char('A' + j);
                first = false;
            }
        }
        cout << "}\n";
        
        cout << "  Mask: ";
        for (int j = n - 1; j >= 0; --j) {
            cout << ((currMask & (1 << j)) ? "1" : "0");
        }
        cout << " (" << currMask << ")\n";
        
        cout << "- Next City Chosen: " << char('A' + nextCity) << "\n";
        cout << "+ Cost Added: " << stepCost << "\n";
        cout << "Total Cost So Far: " << runningCost << "\n";
        
        path.push_back(nextCity);
        currMask |= (1 << nextCity); // Mark the next city as visited in the mask
        currPos = nextCity;
    }
    
    int finalCost = dist[currPos][0];
    runningCost += finalCost;
    
    cout << "--------------------------------------\n";
    cout << "Step " << step << "\n";
    cout << "> Current City: " << char('A' + currPos) << "\n";
    cout << "  Visited: {";
    bool first = true;
    for (int j = 0; j < n; ++j) {
        if (currMask & (1 << j)) {
            if (!first) cout << ", ";
            cout << char('A' + j);
            first = false;
        }
    }
    cout << "}\n";
    
    cout << "  Mask: ";
    for (int j = n - 1; j >= 0; --j) {
        cout << ((currMask & (1 << j)) ? "1" : "0");
    }
    cout << " (" << currMask << ")\n";
        
    cout << "- Next City Chosen: A (Return to Start)\n";
    cout << "+ Cost Added: " << finalCost << "\n";
    cout << "Total Cost So Far: " << runningCost << "\n";
    cout << "--------------------------------------\n\n";
    
    path.push_back(0); // Return to start
    
    return {path, minCost};
}


// Recursive helper function executing Memoization and State-Space Search
// dist: The distance matrix.
// mask: A bitmask representing the set of cities that have been visited so far. 
//       The ith bit is 1 if city i is visited, 0 otherwise.
// pos: The current city we are at.
// dp: 2D table caching previously computed state costs.
// parent: 2D table remembering optimal decisions for path reconstruction.
int tspHelper(const vector<vector<int>>& dist, int mask, int pos, vector<vector<int>>& dp, vector<vector<int>>& parent) {
    int n = dist.size();
    
    // Base Case: If all cities have been visited (mask has all N bits set to 1)
    if (mask == ((1 << n) - 1)) {
        return dist[pos][0]; // TSP rule: Return the cost to traverse from the last city back to start (city 0)
    }
    
    // Memoization check: If we have already solved this sub-problem geometry (exact mask and pos), 
    // simply pull the cached result from our DP table rather than heavily branching.
    if (dp[mask][pos] != -1) {
        return dp[mask][pos];
    }
    
    int ans = 1e9; // Initialize to an imposingly large value logically representing "infinity"
    int bestNextCity = -1;
    
    // State Transitions: Try visiting all other conceptually unvisited cities
    for (int city = 0; city < n; ++city) {
        // If the 'city' bit is 0 in the mask AND we're looking at a different city, it means it's unvisited
        if ((mask & (1 << city)) == 0) {
            
            // New progressive state is formed by flipping on the 'city' bit using Bitwise OR (|)
            // Recursively calculate the ultimate cost from the newly minted state, plus the transition cost
            int newAns = dist[pos][city] + tspHelper(dist, mask | (1 << city), city, dp, parent);
            
            // If this particular transition ends up being strictly better, update recorded optimal answer
            // and save the chosen branching city so we construct the physical path out of it later.
            if (newAns < ans) {
                ans = newAns;
                bestNextCity = city;
            }
        }
    }
    
    // Commit the best optimal route decision from our active state configuration
    parent[mask][pos] = bestNextCity;
    
    // Safely catalogue our minimized cost parameter back into the respective DP compartment, and return
    return dp[mask][pos] = ans;
}
