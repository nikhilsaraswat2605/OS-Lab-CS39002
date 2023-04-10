#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <set>
#include <pthread.h>
#include <mutex>
#include <unistd.h>

using namespace std;

const int MAX_NODES = 37800; // actual 37700
const int MAX_EDGES = 2*290000; // actual 289003

const string kEdgesFilePath = "musae_git_edges.csv";
const int kNumReadPostThreads = 10;
const int kNumPushUpdateThreads = 25;

// Constants
#define NUM_ACTIONS 10

// Function to calculate the log base 2 of a number
int log2(int n) {
    int log = 0;
    while (n >>= 1) ++log;
    return log;
}


// Struct to represent an action
typedef struct {
    int user_id; // node id
    int action_id; // 4th like, 5th comment ,etc
    int action_type; // post ,like , comment
    time_t timestamp;
} Action;


// Node struct to store the information of a graph node
typedef struct Node {
    int degree;
    int head;
    int id;

    pthread_mutex_t feedQueueMutex;
    pthread_cond_t condVar;
    
    vector<Action> wallQueue;
    vector<Action> feedQueue;

    int count[3]; // 0: post, 1: like, 2: comment
    int priority; // 1: chronological, 0: popularity

    Node(int id=0) {
        
        this->degree = 0;
        this->head = -1;
    
        this->id = id;
        this->count[0] = 0;
        this->count[1] = 0;
        this->count[2] = 0;

        this->priority = rand() % 2;

        pthread_mutex_init(&feedQueueMutex, NULL);
        pthread_cond_init(&condVar, NULL);
    }
} Node;

typedef struct EdgeData
{
    int to, cost, flow;
    int nxt; /* nxt: is the index of the next edge */
}EdgeData;

typedef struct Graph
{
    int node_count, edge_count;
    Node nodes[MAX_NODES];
    EdgeData edges[MAX_EDGES];
    Graph(int n)
    {
        node_count = n;
        for (int i = 0; i < n; i++)
        {
            nodes[i].id = i;
        }
        edge_count = 0;
    }
    void addEdge(int from, int to, int cost)
    {
        edges[edge_count].to = to;
        edges[edge_count].cost = cost;
        edges[edge_count].nxt = nodes[from].head;
        nodes[from].head = edge_count;
        nodes[from].degree++;
        edge_count++;
    }
    ~Graph()
    {
        for (int i = 0; i < node_count; i++)
        {
            pthread_mutex_destroy(&nodes[i].feedQueueMutex);
            pthread_cond_destroy(&nodes[i].condVar);
        }
    }
}Graph;

Graph graph(MAX_NODES);
vector<Action> pushQueue[kNumPushUpdateThreads];
set<int> feedQueueSet[kNumReadPostThreads];

// Function to generate a random action
Action generate_action(int user_id) {
    Action action;
    action.user_id = user_id;
    action.action_type = rand() % 3;
    action.action_id = ++graph.nodes[user_id].count[action.action_type];
    time(&action.timestamp);
    return action;
}


pthread_mutex_t push_queue_mutex[kNumPushUpdateThreads];
pthread_mutex_t log_mutex;
pthread_mutex_t log_mutex_terminal;
pthread_mutex_t feed_queue_set_mutex[kNumReadPostThreads];

pthread_cond_t push_queue_cond[kNumPushUpdateThreads];
pthread_cond_t feed_queue_set_cond[kNumReadPostThreads];

// Function to log messages to file and stdout
void log_message(string message) {
    // Open log file
    ofstream log_file;
    log_file.open("sns.log", ios::app);
    if (!log_file.is_open()) {
        cout<<"Error opening log file"<<endl;
        exit(1);
    }

    pthread_mutex_lock(&log_mutex_terminal);
    // start of critical section
    cout<<message;
    // end of critical section
    pthread_mutex_unlock(&log_mutex_terminal);
    
    pthread_mutex_lock(&log_mutex);
    // start of critical section
    log_file<<message;
    // end of critical section
    pthread_mutex_unlock(&log_mutex);

    log_file.close();
}

// Function to simulate user actions
void* UserSimulator(void* arg) {
    
    while(1){
        // Choose random nodes
        const int cnt = 100; // 100 nodes
        int nodes[cnt];
        set<int> node_set;
        while(node_set.size() < cnt) {
            node_set.insert(rand() % 37700);
        }
        int i = 0;
        for (auto it = node_set.begin(); it != node_set.end(); it++) {
            nodes[i++] = *it;
        }
        // Generate actions for each node
        for (int i = 0; i < cnt; i++) {

            // Generate random number of actions depending on degree of node
            int node_id = nodes[i];
            int num_actions = NUM_ACTIONS * (1+log2(graph.nodes[node_id].degree));

            // Log number of actions generated
            string message = "User Simulator thread : Generating " + to_string(num_actions) + " actions for node " + to_string(node_id) + " with degree " + to_string(graph.nodes[node_id].degree) + "\n";
            log_message(message);

            string tmp;
            // Generate actions
            for (int j = 0; j < num_actions; j++) {
                
                // Generate and push action to wall queue
                Action action = generate_action(node_id);
                graph.nodes[node_id].wallQueue.push_back(action);

                int push_queue_id = rand() % 25;
                pthread_mutex_lock(&push_queue_mutex[push_queue_id]);
                // critical section start                
                pushQueue[push_queue_id].push_back(action); // Push action to push queue
                // critical section end
                pthread_mutex_unlock(&push_queue_mutex[push_queue_id]);
                // Signal push queue semaphore
                pthread_cond_signal(&push_queue_cond[push_queue_id]);
                
                
                // Log generated action
                string message ="";
                if(action.action_type == 1){
                    message += "User Simulator thread : Generated action '" + to_string(action.action_id) + "' of type 'like' for node '" + to_string(action.user_id) + "' at time " + ctime(&action.timestamp);
                }else if(action.action_type == 2){
                    message += "User Simulator thread : Generated action '" + to_string(action.action_id) + "' of type 'comment' for node '" + to_string(action.user_id) + "' at time " + ctime(&action.timestamp);
                }else{
                    message += "User Simulator thread : Generated action '" + to_string(action.action_id) + "' of type 'post' for node '" + to_string(action.user_id) + "' at time " + ctime(&action.timestamp);
                }
                tmp+=message;
            }
            log_message(tmp);
        }
        sleep(120);
    }
    pthread_exit(0);
}

int num_read_post_threads[kNumReadPostThreads];
int num_push_update_threads[kNumPushUpdateThreads];
void* push_update(void* arg) {
    int num = (*(int*)arg)+1;
    while(1) {
        
        // pop action from push queue
        pthread_mutex_lock(&push_queue_mutex[num-1]);
        // critical section start
        while(pushQueue[num-1].size() == 0) { // wait for push queue to have elements
            pthread_cond_wait(&push_queue_cond[num-1], &push_queue_mutex[num-1]);
        }
        Action action = pushQueue[num-1].back();
        pushQueue[num-1].pop_back();
        // critical section end
        pthread_mutex_unlock(&push_queue_mutex[num-1]);

        // push action to feed queue of all followers
        int node_id = action.user_id;
        for(int i= graph.nodes[node_id].head; i != -1; i = graph.edges[i].nxt) {
            int neighbour_id = graph.edges[i].to;

            pthread_mutex_lock(&graph.nodes[neighbour_id].feedQueueMutex);
            // critical section start
            graph.nodes[neighbour_id].feedQueue.push_back(action);
            // critical section end
            pthread_mutex_unlock(&graph.nodes[neighbour_id].feedQueueMutex);
            // Signal feed queue mutex
            // pthread_cond_signal(&graph.nodes[neighbour_id].condVar);

            int feed_queue_set_id = rand() % 10;
            pthread_mutex_lock(&feed_queue_set_mutex[feed_queue_set_id]);
            // critical section start
            feedQueueSet[feed_queue_set_id].insert(neighbour_id);
            // critical section end
            pthread_mutex_unlock(&feed_queue_set_mutex[feed_queue_set_id]);
            // Signal feed queue set mutex
            pthread_cond_signal(&feed_queue_set_cond[feed_queue_set_id]);
        }


        // Log pushed action
        string message;
        if(action.action_type == 0){
            message = "pushUpdate thread '" + to_string(num) + "' ('" + to_string(pthread_self()) + "') : pushed action '" + to_string(action.action_id) + "' of type 'post' for node '" + to_string(action.user_id) + "' at time " + ctime(&action.timestamp);
        }else if(action.action_type == 1){
            message = "pushUpdate thread '" + to_string(num) + "' ('" + to_string(pthread_self()) + "') : pushed action '" + to_string(action.action_id) + "' of type 'like' for node '" + to_string(action.user_id) + "' at time " + ctime(&action.timestamp);
        }else{
            message = "pushUpdate thread '" + to_string(num) + "' ('" + to_string(pthread_self()) + "') : pushed action '" + to_string(action.action_id) + "' of type 'comment' for node '" + to_string(action.user_id) + "' at time " + ctime(&action.timestamp);
        }
        log_message(message);

    }
    pthread_exit(0);
}

int mutual_friends[kNumReadPostThreads][MAX_NODES];
void* read_post(void* arg) {
    int num = (*(int*)arg)+1;
    while(1){
        pthread_mutex_lock(&feed_queue_set_mutex[num-1]);
        // critical section start
        while(feedQueueSet[num-1].empty()) { // wait for feed queue set to have elements
            pthread_cond_wait(&feed_queue_set_cond[num-1], &feed_queue_set_mutex[num-1]);
        }
        int node_id = *feedQueueSet[num-1].begin();
        feedQueueSet[num-1].erase(feedQueueSet[num-1].begin());
        // critical section end
        pthread_mutex_unlock(&feed_queue_set_mutex[num-1]);

        vector<Action> actions_tmp;
        // pop action from feed queue
        pthread_mutex_lock(&graph.nodes[node_id].feedQueueMutex);
        // critical section start
        int tmp_priority = graph.nodes[node_id].priority;
        while(graph.nodes[node_id].feedQueue.size() > 0 ){
            Action action = graph.nodes[node_id].feedQueue.back();
            graph.nodes[node_id].feedQueue.pop_back();
            actions_tmp.push_back(action);
        }
        // critical section end
        pthread_mutex_unlock(&graph.nodes[node_id].feedQueueMutex);

        

        if(tmp_priority == 1){ // chronological
            sort(actions_tmp.begin(), actions_tmp.end(), 
            [](Action a, Action b) {
                return a.timestamp < b.timestamp;
            });
        }else{ // popularity
        
            // find mutual friends 
            for(int i=0; i<actions_tmp.size(); i++){
                int user_id = actions_tmp[i].user_id;
                set<int> st;
                for(int j= graph.nodes[node_id].head; j != -1; j = graph.edges[j].nxt) {
                    int neighbour_id = graph.edges[j].to;
                    st.insert(neighbour_id);
                }
                int cnt=0;
                for(int j= graph.nodes[user_id].head; j != -1; j = graph.edges[j].nxt) {
                    int neighbour_id = graph.edges[j].to;
                    if(st.find(neighbour_id) != st.end()){
                        cnt++;
                    }
                }
                mutual_friends[num-1][user_id] = cnt;
            }

            sort(actions_tmp.begin(), actions_tmp.end(),  
            [num](Action a, Action b) {
                return mutual_friends[num-1][a.user_id] > mutual_friends[num-1][b.user_id];
            });
        }
        
        // Log read action
        string tmp;
        for(int i=0; i<actions_tmp.size(); i++){
            string message;
            // print messages of the form “I read action number XX of type YY posted by user ZZ at time TT”
            message = "readPost thread '" + to_string(num) + "' ('" + to_string(pthread_self()) + "') : I read action '" + to_string(actions_tmp[i].action_id) + "' of type '" + (actions_tmp[i].action_type == 0 ? "post" : actions_tmp[i].action_type == 1 ? "like" : "comment") + "' posted by user '" + to_string(actions_tmp[i].user_id) + "' at time " + ctime(&actions_tmp[i].timestamp);
            tmp += message;
        }
        log_message(tmp);
    }
     pthread_exit(0);
}

// Read the graph from file and initialize the graph vector
void* main_process(void* arg) {
    // clear the sns.log file
    ofstream ofs;
    ofs.open("sns.log", ofstream::out | ofstream::trunc);
    ofs.close();


    // open the file
    ifstream file(kEdgesFilePath);

    // check if the file is opened successfully
    if (!file.is_open()) {
        cerr << "Could not open file: " << kEdgesFilePath << endl;
        exit(1);
    }

    // skip the first line
    string line;
    file>>line;

    // read the rest of the file
    int src, dest;
    while (file>>line) {    
        // parse the line
        int pos = line.find(',');
        src = stoi(line.substr(0, pos));
        dest = stoi(line.substr(pos + 1));
        
        // add the edge to the graph
        graph.addEdge(src, dest, 1);
        graph.addEdge(dest, src, 1);

    }

    // close the file
    file.close();
    
    return NULL;
}

int main() {
    
    // Seed random number generator
    srand(time(NULL));

    // Initialize mutexes
    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&log_mutex_terminal, NULL);
    for(int i=0; i<kNumReadPostThreads; i++){
        pthread_mutex_init(&feed_queue_set_mutex[i], NULL);
    }
    for(int i=0; i<kNumPushUpdateThreads; i++){
        pthread_mutex_init(&push_queue_mutex[i], NULL);
    }

    // Initialize condition variables
    for(int i=0; i<kNumReadPostThreads; i++){
        pthread_cond_init(&feed_queue_set_cond[i], NULL);
    }
    for(int i=0; i<kNumPushUpdateThreads; i++){
        pthread_cond_init(&push_queue_cond[i], NULL);
    }

    // main process should read the input file and create the graph
    pthread_t mainProcessThread;
    pthread_attr_t attr_main;
    pthread_attr_init(&attr_main);
    pthread_create(&mainProcessThread, &attr_main, main_process, NULL);

    // Wait for main process to finish
    pthread_join(mainProcessThread, NULL);

    // Create a userSimulator thread using pthread library
    pthread_t userSimulatorThread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&userSimulatorThread, &attr, UserSimulator, NULL);
    
    // Create a pool of 25 pushUpdate threads using pthread library
    pthread_t pushUpdateThreads[kNumPushUpdateThreads];
    pthread_attr_t attr2[kNumPushUpdateThreads];
    for (int i = 0; i < kNumPushUpdateThreads; i++) {
        pthread_attr_init(&attr2[i]);
    }
    for (int i = 0; i < kNumPushUpdateThreads; i++) {
        num_push_update_threads[i] = i;
    }
    for (int i = 0; i < kNumPushUpdateThreads; i++) {
        pthread_create(&pushUpdateThreads[i], &attr2[i], push_update, &num_push_update_threads[i]);
    }
    
    // Create a pool of 10 readPost threads using pthread library
    pthread_t readPostThreads[kNumReadPostThreads];
    pthread_attr_t attr1[kNumReadPostThreads];
    for (int i = 0; i < kNumReadPostThreads; i++) {
        pthread_attr_init(&attr1[i]);
    }
    for (int i = 0; i < kNumReadPostThreads; i++) {
        num_read_post_threads[i] = i;
    }
    for (int i = 0; i < kNumReadPostThreads; i++) {
        pthread_create(&readPostThreads[i], &attr1[i], read_post, &num_read_post_threads[i]);
    }

    // Wait for the threads to finish
    pthread_join(userSimulatorThread, NULL);

    for (int i = 0; i < kNumReadPostThreads; i++) {
        pthread_join(readPostThreads[i], NULL);
    }

    for (int i = 0; i < kNumPushUpdateThreads; i++) {
        pthread_join(pushUpdateThreads[i], NULL);
    }

    // destroy mutex
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&log_mutex_terminal);
    for(int i=0; i<kNumReadPostThreads; i++){
        pthread_mutex_destroy(&feed_queue_set_mutex[i]);
    }
    for(int i=0; i<kNumPushUpdateThreads; i++){
        pthread_mutex_destroy(&push_queue_mutex[i]);
    }
    
    
    
    // destroy condition variables
    for(int i=0; i<kNumReadPostThreads; i++){
        pthread_cond_destroy(&feed_queue_set_cond[i]);
    }
    for(int i=0; i<kNumPushUpdateThreads; i++){
        pthread_cond_destroy(&push_queue_cond[i]);
    }

    return 0;
}

