config = {
    debug = true, -- Displays log messages, allows wireframe view
    fov = 80, -- Field of view
    player_speed = { -- Speed of the player
        slow = 20,
        normal = 40,
        sprint = 60,
    },
    threads = 4, -- Number of threads to use
    render_radius = 3, -- Render radius of the world (where the player's location is the origin)
    reach = 50.0, -- How far the player can reach to break/place blocks
    mouse_sensitivity = 0.0025 -- Mouse sensitivity
}