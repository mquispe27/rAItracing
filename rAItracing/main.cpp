//
//  main.cpp
//  rAItracing
//
//  Created by Matthew Quispe on 1/18/25.
//

#include <fstream>

#include "constants.h"

#include "bvh.h"
#include "camera.h"
#include "crow_all.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "quad.h"
#include "sphere.h"
#include "texture.h"


std::atomic<int> rendering_progress(0);
std::vector<unsigned char> rendered_image;
std::mutex image_mutex;

void bouncing_spheres() {
    hittable_list world;
    
    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));
    
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
    
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));



    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 20;
    cam.max_depth         = 20;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });

}

void checkered_spheres() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });
}

void earth() {
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,12);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(hittable_list(globe), [](int progress) {
        rendering_progress.store(progress);
    });
    
    // Store the rendered image
    std::lock_guard<std::mutex> lock(image_mutex);
    rendered_image = cam.image_buffer;

    



}

void perlin_spheres() {
    hittable_list world;

    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });
    
    // Store the rendered image
    std::lock_guard<std::mutex> lock(image_mutex);
    rendered_image = cam.image_buffer;
}

void quads() {
    hittable_list world;

    // Materials
    auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });
    
    // Store the rendered image
    std::lock_guard<std::mutex> lock(image_mutex);
    rendered_image = cam.image_buffer;
}

void simple_light() {
    hittable_list world;

    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 20;
    cam.lookfrom = point3(26,3,6);
    cam.lookat   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });
    
    // Store the rendered image
    std::lock_guard<std::mutex> lock(image_mutex);
    rendered_image = cam.image_buffer;
}

void cornell_box() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });
    
    // Store the rendered image
    std::lock_guard<std::mutex> lock(image_mutex);
    rendered_image = cam.image_buffer;
}




void render_scene(const std::string& option) {
    if (option == "bouncing_spheres") {
        bouncing_spheres();
    } else if (option == "checkered_spheres") {
        checkered_spheres();
    } else if (option == "earth") {
        earth();
    } else if (option == "perlin_spheres") {
        perlin_spheres();
    } else if (option == "quads") {
        quads();
    } else if (option == "simple_light") {
        simple_light();
    } else if (option == "cornell_box") {
        cornell_box();
    } else {
        throw std::invalid_argument("Invalid drawing option");
    }

    std::cout << rendered_image.size() << std::endl;
    
    rendering_progress.store(100);

}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>Ray Tracing Options</title>
                <style>
                    
                    :root {
                      --red: rgb(208, 67, 12);
                      --beige: #d4c8a9;
                      --light-beige: #f1e6c9;
                      --light-bg: rgba(255, 255, 255, 28%);
                      --lighter-green: #d4d6b0;
                      --green: #b0b470;
                      --dark-green: #58823a;
                      --darker-green: #43603b;
                      --orange: #d13717;
                      --orange-dark: #b02e0c;
                      --base-bg: #f1f3f5;
                      --light-grey: #b0a892;
                      --medium-grey: #b1a892;
                      --dark-grey: #264528;
                      --pink: #ab5c53;
                      --light-pink: #b28883;
                      --salmon: #f7bfa8;
                      --brown: #6f5436;
                      --dark-brown: #6f5436;
                      --selected-brown: #93876b;
                      --sidebar-brown: #b8a886;
                      --background-brown: #cfbe98;
                      --my-green: #b7bc71;
                    }

                    button.btn-small {
                      font-size: 80%;
                    }

                    button.button-error {
                      color: white;
                      background: var(--red);
                    }

                    a.button {
                      color: #212529;
                      background-color: #f1f3f5;
                      border: 1px solid #dee2e6;
                      border-radius: 0.25em;
                      padding: 0.375em 0.75em;
                      text-decoration: none;
                    }

                    button {
                      font-size: 1em;
                      padding: 0.5em 1em;
                      border-radius: 0.25em;
                      border: 1px solid #dee2e6;
                      background: var(--pink);
                      color: white;
                      cursor: pointer;
                      transition: background 0.3s ease;
                    }

                    button:hover {
                      transform: scale(1.05);
                    }

                    .column {
                      display: flex;
                      flex-direction: column;
                      justify-content: center;
                      align-items: center;
                      gap: 1em;
                    }
                    body {
                        background-color: var(--beige);
                        color: var(--dark-green);
                        min-height: 100vh;
                        padding: 2rem;
                    }

                    .container {
                        background-color: var(--light-bg);
                        padding: 2rem;
                        border-radius: 1rem;
                        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    }

                    .select-wrapper {
                        position: relative;
                        width: 100%;
                    }

                    .styled-select {
                        appearance: none;
                        background-color: var(--light-beige);
                        border: 2px solid var(--dark-green);
                        border-radius: 0.5rem;
                        padding: 0.5rem 1rem;
                        font-size: 1rem;
                        width: 100%;
                        cursor: pointer;
                    }

                    .styled-select:focus {
                        outline: none;
                        box-shadow: 0 0 0 2px var(--green);
                    }

                    .styled-select::after {
                        content: '\25BC';
                        position: absolute;
                        top: 50%;
                        right: 1rem;
                        transform: translateY(-50%);
                        pointer-events: none;
                    }

                    .render-btn {
                        background-color: var(--dark-green);
                        color: var(--light-beige);
                        border: none;
                        padding: 0.75rem 1.5rem;
                        font-size: 1rem;
                        border-radius: 0.5rem;
                        cursor: pointer;
                        transition: background-color 0.3s ease, transform 0.2s ease;
                    }

                    .render-btn:hover {
                        background-color: var(--darker-green);
                        transform: translateY(-2px);
                    }

                    .progress-container {
                        width: 100%;
                        margin-top: 1rem;
                        display: flex;
                        align-items: center;
                    }

                    #progressBar {
                        flex-grow: 1;
                        height: 1rem;
                        border-radius: 0.5rem;
                        overflow: hidden;
                    }

                    #progressBar::-webkit-progress-bar {
                        background-color: var(--light-beige);
                    }

                    #progressBar::-webkit-progress-value {
                        background-color: var(--dark-green);
                    }

                    #progressText {
                        margin-left: 1rem;
                        font-weight: bold;
                        color: var(--dark-green);
                    }

                    .image-container {
                        margin-top: 2rem;
                        max-width: 100%;
                        overflow: hidden;
                        border-radius: 1rem;
                        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    }

                    #renderedImage {
                        width: 100%;
                        height: auto;
                        display: block;
                    }
                </style>
                <script>
                    function renderScene() {
                        const selectedOption = document.getElementById("drawingOptions").value;
                        fetch("/render", {
                            method: "POST",
                            headers: {
                                "Content-Type": "application/json"
                            },
                            body: JSON.stringify({ prompt: selectedOption })
                        })
                        .then(response => {
                            if (response.ok) {
                                const interval = setInterval(() => {
                                    fetch("/progress")
                                    .then(response => response.json())
                                    .then(data => {
                                        document.getElementById("progressBar").value = data.progress;
                                        document.getElementById("progressText").textContent = `${Math.round(data.progress)}%`;
                                        if (data.progress >= 100) {
                                            clearInterval(interval);
                                            document.getElementById("renderedImage").src = "/image?" + new Date().getTime();
                                        }
                                    });
                                }, 1000);
                            } else {
                                alert("Error initiating rendering");
                            }
                        });
                    }
                </script>
            </head>
            <body class="column">
                <h1>Ray Tracing Renderer</h1>
                <div class="container">
                    <div class="select-wrapper">
                        <select id="drawingOptions" class="styled-select">
                            <option value="bouncing_spheres">Bouncing Spheres</option>
                            <option value="checkered_spheres">Checkered Spheres</option>
                            <option value="earth">Earth</option>
                            <option value="perlin_spheres">Perlin Spheres</option>
                            <option value="quads">Quads</option>
                            <option value="simple_light">Simple Light</option>
                            <option value="cornell_box">Cornell Box</option>
                        </select>
                    </div>
                    <button onclick=renderScene() class="render-btn">Render Scene</button>
                </div>
                <div class="progress-container">
                    <progress id="progressBar" value="0" max="100"></progress>
                    <span id="progressText">0%</span>
                </div>
                <div class="image-container">
                    <img id="renderedImage" src="" alt="Rendered Image" />
                </div>
            </body>
            </html>
        )";
    });

    CROW_ROUTE(app, "/progress").methods("GET"_method)
    ([](){
        return crow::json::wvalue{{"progress", rendering_progress.load()}};
    });

    CROW_ROUTE(app, "/image").methods("GET"_method)
    ([](){
//        std::lock_guard<std::mutex> lock(image_mutex);
//        crow::response res;
//        res.set_header("Content-Type", "image/png");
//        res.body = std::string(reinterpret_cast<const char*>(rendered_image.data()), rendered_image.size());
//        return res;
//        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
//        
        crow::response res;
        res.set_static_file_info("user_image.jpg");
        res.set_header("Content-Type", "image/jpeg");
        return res;
    });


    CROW_ROUTE(app, "/render").methods("POST"_method)
    ([](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);

        std::string prompt = x["prompt"].s();
        std::thread(render_scene, prompt).detach();

        return crow::response(200, "Rendering initiated");
    });

    app.port(8080).multithreaded().run();
}
