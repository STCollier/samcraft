config = {
    debug = true, -- Displays log messages, allows wireframe view
    fov = 80, -- Field of view
    player_speed = { -- Speed of the player
        slow = 3,
        normal = 4.5,
        sprint = 6.5,
    },
    threads = 4, -- Number of threads to use
    render_radius = 5, -- Render radius of the world (where the player's location is the origin)
    reach = 50.0, -- How far the player can reach to break/place blocks
    mouse_sensitivity = 0.25 -- Mouse sensitivity
}