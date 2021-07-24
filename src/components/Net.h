#include "Component.h"
namespace pong
{
    class Net : public Component
    {
    public:
        int pID;
        Net(int pID)
        {
            tag = tags::indep;
            debug_type = "net component";
            this->pID = pID;
        }
        ~Net() {}
    };
}