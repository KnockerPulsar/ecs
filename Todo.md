
# FIXMEs
- FIXME:?? If you hit the ball with the side of the paddle, the ball might get stuck inside the paddle.

# TODOs
- TODO: Figure out a way to not dynamically allocate while building a quadtree.
  - One way we could go about this is to:
    1. Add a `used` flag in the quadtree class.
    2. During initialization, recursively allocate quadtrees and mark them all as not used.
    3. While inserting, instead of allocating a quadtree, mark it as used.
    4. While checking, instead of checking for nulls, check for used.
    5. While destroying, just mark as unused.
    6. Create a de-initializer function that actually de-allocates the quadtree during program shutdown.
  - This still doesn't solve the problem of dynamically allocating position vectors though...  
    The main issue is that collisions depend on their entity's position to position themselves in the world,
    but quadtree collisions are not attached to any entity. 
    My best solution would be to keep an array of allocated
    `Vector2`'s, add to the array as we're initially creating the quadtree, then delete the array during de-initialization.
    This way, we can make sure the positions outlive their collisions at the very least.

- TODO: Better particle system, probably based on the one from [here](https://youtu.be/A0-UOZ2v4V8)
- 
- TODO: Iterator invalidation when deleting components or entities while iterating over a vector of them. (Not a big issue right now)

- TODO: Instead of remaking the whole quadtree for a mostly static scene, we can mark each collider as "static" or
"dynamic", then we only need to rebuild the nodes for dynamic colliders

- TODO: More TODO's

# DONE
  - Remove events from the queue/vector on completion.
  - Make adding components to entities at compile time easier (through non-type arguments entt.AddComponents<comp1, comp2,...>())
    Kinda done by adding chaining. Not sure how one would do the template stuff, but I think that would at least require default constructors.
  - Improve on the "event" system
    Add repeated calls with delay (foo() every 0.5 seconds for example)
    or calls that will be called a certain number of frames/time. (foo() for every frame for 0.5 seconds for example)
    Take into account if the event should repeat for the event queue sorting function so that repeated events are not popped.
    Added a repitition count for events. Reptitions will have the same delay as the original call. 



# Meh
  - Convert the event vector to a queue that's sorted by delay.
    Currently no real need for this since we already remove events at the end of their lifetimes
  - Drawing all particles to a texture first then displaying that on screen, might be better than lots of drawcalls?
    Seems that raylib already takes care of that with batching. [Github issue discussing batching](https://github.com/raysan5/raylib/issues/267)
  - 2D layers
    Not really needed right now, can be done through `DrawTexture()`. [More info](https://github.com/raysan5/raylib/issues/682)
  - Implement "ephemeral" and "persistent" scenes
    I think what I originally meant was scenes that were to be removed from memory on transition. I don't think that's quite need right now.
  - Maybe remove system and event checking from the scene code, leaving just the `Update()` function for custom behaviour?
    Maybe later if needed for some reason. Currently, this is not really needed. I think with the `Update()` approach, we can have another internal function that handles system and event checking, then calls `Update()` if we really need custom behaviour.
  - Improve the "scene" system
    No idea what I meant by this. 
  
