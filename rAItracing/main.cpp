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
    auto earth_texture = make_shared<image_texture>("/Users/mquispe/Documents/IAP 2025/cpp-projects/rAItracing/rAItracing/earthmap.jpg");
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


}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>Ray Tracing Options</title>
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
                                        if (data.progress >= 100) {
                                            clearInterval(interval);
                                            document.getElementById("renderedImage").src = "/image";
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
            <body>
                <h1>Select a Drawing Option</h1>
                <select id="drawingOptions">
                    <option value="bouncing_spheres">Bouncing Spheres</option>
                    <option value="checkered_spheres">Checkered Spheres</option>
                    <option value="earth">Earth</option>
                    <option value="perlin_spheres">Perlin Spheres</option>
                    <option value="quads">Quads</option>
                    <option value="simple_light">Simple Light</option>
                    <option value="cornell_box">Cornell Box</option>
                </select>
                <button onclick=renderScene()>Render</button>
                <progress id="progressBar" value="0" max="100"></progress>
                <img id="renderedImage" src="" alt="Rendered Image" />
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
        std::lock_guard<std::mutex> lock(image_mutex);
        crow::response res;
        res.set_header("Content-Type", "image/png");
        res.body = std::string(reinterpret_cast<const char*>(rendered_image.data()), rendered_image.size());
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
