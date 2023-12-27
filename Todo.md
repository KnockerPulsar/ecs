TODO: fix crash when accessing currentLevel without adding levels / setting the start level
TODO: Start implementing the main scene (particle)
TODO: Difficulty menu for AI
TODO: Delay the ball respawn
TODO: Make systems accept `Resources` instead of separate local and global resource parameters to reduce code complexity
    ```
        struct Resources {
            LevelResources& level;
            GlobalResources& global;
            ... Whatever other data is needed (events, etc...)
        }
    ```
