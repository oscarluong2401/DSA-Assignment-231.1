#include "main.h"
const int maxInt = 2147483647;
class imp_res : public Restaurant{
public:
    class CDLL{ // circular doubly linked list
    public:
        customer *X; // a changeable head for updating X position
        int count;
        CDLL(): X(nullptr), count(0) {}
        ~CDLL() {
            while (X) removeAt(X);
            delete X;
        }
        void add(customer *c){ // directly add c to the updated X position, change head to that position
            if (count >= MAXSIZE/2){
                customer *temp = X; int maxDiff = 0;
                for (int i = 0; i < count; i++, temp = temp->next){
                    if (abs(temp->energy - c->energy) > maxDiff){
                        maxDiff = abs(temp->energy - c->energy); X = temp;
                    }
                }
            }
            if (!X) { X = c; X->next = X->prev = c; }
            else {
                if (c->energy < X->energy){ c->prev = X->prev; c->next = X; }
                else { c->prev = X; c->next = X->next; }
                X = c->next->prev = c->prev->next = c;
            } count++;
        }
        void removeAt(customer* pos) { // delete chosen pos, update head to either side
            pos->next->prev = pos->prev; pos->prev->next = pos->next;
            X = pos->energy > 0 ? pos->next : pos->prev;
            delete pos; if (--count == 0) X = nullptr;
        }
    };
    class Q{
    public:
        customer *head, *tail; // doubly linked list, be careful to Shell sort on this
        int count;
        Q(): head(nullptr), tail(nullptr), count(0) {}
        ~Q(){
            while (head) removeAt(head);
            delete head;
        }
        void enqueue(customer *c){ // add to tail
            if (!head) head = tail = c;
            else { c->prev = tail; tail = tail->next = c; }
            count++;
        }
        customer* dequeue(){ // take the head of the queue, can be used to put in table
            customer* pos = head; head = head->next;
            if (count == 1) tail = nullptr; else head->prev = nullptr;
            count--; return pos; // pos->next will be pointed to someone on the table soon, don't worry
        }
        void removeAt(customer *pos){ // delete at required position, this is the generalized version
            if (pos == head) head = head->next; else pos->prev->next = pos->next;
            if (pos == tail) tail = tail->prev; else pos->next->prev = pos->prev;
            count--; delete pos;
        }
    };
    bool isDuplicated(const string& name) const{
        for (customer *temp = tList->head; temp; temp = temp->next){
            if (temp->name == name) return true;
        } return false;
    }
    static customer* getNext(customer *curr, int gap){
        customer *temp = curr;
        for (int i = 0; i < gap; i++) temp = temp->next;
        return temp;
    }
    static void swap(customer *c1, customer*c2){
        string tempN = c1->name; int tempE = c1->energy;
        c1->name = c2->name; c1->energy = c2->energy;
        c2->name = tempN; c2->energy = tempE;
    }
    static customer* found(const string& name, customer *head, int size){
        customer *temp = head;
        for (int i = 0; i < size && temp; i++, temp = temp->next){ // fix this for priority in timeList
            if (temp->name == name) return temp;
        } return nullptr;
    }
    int absInsSort(customer *currHead, int size, int gap) const{
        int result = 0;
        for (int i = gap; i < size; i+=gap){
            for (int j = i; j >= gap; j-=gap){
                customer *front = getNext(currHead,j-gap), *back = getNext(currHead,j);
                // if front has smaller energy, or equal |RES| but arrive later, swap back
                if (abs(back->energy) > abs(front->energy) || (abs(back->energy) == abs(front->energy) && found(front->name, found(back->name, tList->head, tList->count), tList->count)))
                {swap(front, back); result++;}
            }
        } return result;
    }
    int absShellSort(customer* currHead, int size) const{
        int result = 0;
        for (int i = size/2; i>0; i/=2){
            if (i != 2) for (int j=0; j<i; j++) result += absInsSort(getNext(currHead, j), size-j, i);
        } return result;
    }
    // attributes and functions of imp_res officially start here
    CDLL* rTable; Q* rQueue; Q* tList;
    imp_res(): rTable(new CDLL()), rQueue(new Q()), tList(new Q()) {}
    ~imp_res(){ delete rTable; delete rQueue; delete tList; }
    void RED(string name, int energy) override{
        if (energy == 0 || rQueue->count == MAXSIZE || isDuplicated(name)) return ;
        auto *c = new customer (name, energy, nullptr, nullptr);
        if (rTable->count < MAXSIZE) rTable->add(c); else rQueue->enqueue(c);
        tList->enqueue(new customer(name, energy, nullptr, nullptr));
    }
    void BLUE(int num) override{
        for (int i = 0; i < num && rTable->X;){
            customer *temp = tList->head; // PURPLE can push the person at the top of the tList further back in the rQueue
            while (temp){
                customer *pos = found(temp->name, rTable->X, rTable->count);
                if (pos) {
                    rTable->removeAt(pos); tList->removeAt(temp);
                    i++; break;
                } else temp = temp->next;
            }
        }
        while (rTable->count < MAXSIZE && rQueue->count > 0) rTable->add(rQueue->dequeue());
    }
    void PURPLE() override{
        int maxIdx = 0, maxAbsEnergy = 0; customer *temp = rQueue->head;
        for (int i = 0; i < rQueue->count; i++, temp = temp->next){
            if (abs(temp->energy) >= maxAbsEnergy){
                maxIdx = i; maxAbsEnergy = abs(temp->energy);
            }
        }
        return BLUE(absShellSort(rQueue->head, maxIdx+1) % MAXSIZE);
    }
    void REVERSAL() override{
        if (!(rTable->X)) return ;
        string tempName = rTable->X->name;
        customer *first = rTable->X, *last = first->next; // 2 pointers like Mr Phung's quick sort
        while (true) {
            while (first->energy < 0 && first != last) first = first->prev;
            while (last->energy < 0 && first != last) last = last->next;
            swap(first, last); if (first == last || first->prev == last) break;
            first = first->prev; last = last->next;
        }
        first = rTable->X, last = first->next;
        while (true) {
            while (first->energy > 0 && first != last) first = first->prev;
            while (last->energy > 0 && first != last) last = last->next;
            swap(first, last); if (first == last || first->prev == last) break;
            first = first->prev; last = last->next;
        }
        rTable->X = found(tempName, rTable->X, rTable->count);
    }
    void UNLIMITED_VOID() override{
        if (rTable->count < 4) return ;
        customer *subHead = rTable->X, *subTail = getNext(subHead,2), *minHead = subHead; // setup for finding the max-sum-sublist
        int maxLength = 0, minSum = maxInt, currSum = subHead->energy + subHead->next->energy + subHead->next->next->energy; // sum of first 3, next one will be added in loop;
        for (int currLength = 4; currLength <= rTable->count; currLength++){
            subTail = subTail->next; currSum += subTail->energy; // increase length, now the currSum is the sum of at least 4 elements
            for (int i = 0; i < rTable->count; i++, subHead = subHead->next, subTail = subTail->next){
                if (currSum <= minSum){ maxLength = currLength; minHead = subHead; minSum = currSum; }
                currSum = currSum - subHead->energy + subTail->next->energy;
            }
        } // having obtained the final subHead and maxLength, let's find the lowest energy
        customer *temp = minHead; int minEnergy = maxInt, minRelativeIdx = 0;
        for (int i = 0; i < maxLength; i++, temp = temp->next){
            if (temp->energy < minEnergy){
                minEnergy = temp->energy; minRelativeIdx = i;
            }
        }
        for (int i = 0; i < maxLength; i++) getNext(minHead,(minRelativeIdx + i) % maxLength)->print(); // from minIdx, print clockwise
    }
    void DOMAIN_EXPANSION() override{
        customer *temp = tList->head; int sum = 0;
        while (temp){ sum += temp->energy; temp = temp->next; }
        for (temp = tList->tail; temp; temp = temp->prev){ // printing phase, last to first
            if ((sum >= 0 && temp->energy < 0) || (sum < 0 && temp->energy > 0)) temp->print();
        }
        temp = tList->head;
        while (temp){
            if ((sum >= 0 && temp->energy < 0) || (sum < 0 && temp->energy > 0)){ // kicking phase, first to last
                customer *newTemp = temp->next; customer *pos; // hold the next position for temp to go if temp->name can be found elsewhere
                pos = found(temp->name, rTable->X, rTable->count); if (pos) rTable->removeAt(pos); // must be in either queue or table
                pos = found(temp->name, rQueue->head, rQueue->count); if (pos) rQueue->removeAt(pos); // so just find and delete it
                tList->removeAt(temp); temp = newTemp;
            } else temp = temp->next;
        }
        while (rTable->count < MAXSIZE && rQueue->count > 0) rTable->add(rQueue->dequeue());
    }
    void LIGHT(int num) override{
        customer *temp = (num == 0 ? rQueue->head : rTable->X);
        for (int i = 0; i < (num == 0 ? rQueue->count : rTable->count); i++, temp = (num >= 0 ? temp->next : temp->prev)) temp->print();
    }
};