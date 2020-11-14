#ifndef QUEUEWITHWEIGHT_H
#define QUEUEWITHWEIGHT_H

#include <vector>
#include <queue>

//可用于游戏排队
template <typename T>
class QueueWithWeight
{
    std::vector<unsigned> _weights;
    std::vector<std::queue<T>> _queues;
    std::queue<T> *_major = nullptr;
public:
    QueueWithWeight(const std::vector<unsigned> &weights={})
    {
        if(weights.empty())
        {
            _queues.emplace_back(std::queue<T>());
            _major = &_queues[0];
            return;
        }
        _weights = weights;
        _queues.assign(weights.size(), std::queue<T>());
    }
    bool push(T t, unsigned index=0)
    {
        if(_major)
        {
            _major->push(t);
            return true;
        }
        if(index < _queues.size())
        {
            _queues[index].push(t);
            return true;
        }
        return false;
    }
    bool empty()
    {
        for(const auto &q : _queues)
        {
            if(!q.empty())
                return false;
        }
        return true;
    }
    T pop()
    {
        if(_major)
        {
            T t = _major->front();
            _major->pop();
            return t;
        }
        std::vector<unsigned> weights(_weights);
        unsigned weight_sum=0;
        for(int i=_queues.size(); i; --i)
        {
            if(_queues[i-1].empty())
                weights[i-1] = 0;
            else
                weight_sum += weights[i-1];
        }
        int pos = rand() % weight_sum;
        for(int i=weights.size(); i; --i)
        {
            if(weights[i-1] > pos)
            {
                T t = _queues[i-1].front();
                _queues[i-1].pop();
                return t;
            }
            pos -= weights[i-1];
        }
        return 0;
    }
};

#endif // QUEUEWITHWEIGHT_H
