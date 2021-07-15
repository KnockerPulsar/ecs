namespace pong
{
    class Utils
    {
    public:
        static int GetUniqueID();
    };

    // Note: to make this thread-safe, replace the first line with
    //     static std::atomic<std::uint32_t> uid { 0 };  // <<== initialised
    // https://stackoverflow.com/questions/39447118/thread-safe-unique-id-generation-in-c
     int Utils::GetUniqueID()
    {
        static int uid = 0;
        return uid++;
    }
}
