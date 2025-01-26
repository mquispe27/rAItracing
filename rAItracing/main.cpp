//
//  main.cpp
//  rAItracing
//
//  Created by Matthew Quispe on 1/18/25.
//

#include <fstream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"


#include <string>
#include <regex>
#include <cstdlib>

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

// Struct that holds all custom settings
struct CustomSettings {
    std::string prompt;
    std::optional<double> aspectRatio;
    std::optional<int> imageWidth;
    std::optional<int> samplesPerPixel;
    std::optional<int> maxDepth;
    std::optional<std::array<double, 3>> backgroundColor;
    std::optional<double> vfov;
    std::optional<std::array<double, 3>> lookfrom;
    std::optional<std::array<double, 3>> lookat;
    std::optional<std::array<double, 3>> vup;
    std::optional<double> defocusAngle;
    std::optional<double> focusDist;
    std::optional<int> numSpheres;
    std::optional<int> numQuads;
    std::optional<std::string> response;
};

struct RGB {
    int r, g, b;
};

RGB hexToRGB(std::string hexColor) {
    // Remove '#' if present
    hexColor.erase(std::remove(hexColor.begin(), hexColor.end(), '#'), hexColor.end());

    if (hexColor.length() != 6) {
        return {0, 0, 0}; // Invalid input, return black
    }

    RGB result;
    std::stringstream ss;
    ss << std::hex << hexColor;
    int hexValue;
    ss >> hexValue;

    result.r = (hexValue >> 16) & 0xFF;
    result.g = (hexValue >> 8) & 0xFF;
    result.b = hexValue & 0xFF;

    return result;
}


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

void custom_scene(const CustomSettings& settings) {
    hittable_list world;

    auto numSpheres = settings.numSpheres.value_or(0);
    auto numQuads = settings.numQuads.value_or(0);

    for (int i = 0; i < numSpheres; ++i) {
        auto radius = random_double(0.1, 5.0);
        auto x = random_double(-5.0, 5.0);
        auto y = random_double(-5.0, 5.0);
        auto z = random_double(-5.0, 5.0);
        auto color_ = color(random_double(), random_double(), random_double());
        world.add(make_shared<sphere>(point3(x, y, z), radius, make_shared<lambertian>(color_)));
    }

    for (int i = 0; i < numQuads; ++i) {
        auto x1 = random_double(-5.0, 5.0);
        auto y1 = random_double(-5.0, 5.0);
        auto z1 = random_double(-5.0, 5.0);
        auto x2 = random_double(-5.0, 5.0);
        auto y2 = random_double(-5.0, 5.0);
        auto z2 = random_double(-5.0, 5.0);
        auto x3 = random_double(-5.0, 5.0);
        auto y3 = random_double(-5.0, 5.0);
        auto z3 = random_double(-5.0, 5.0);
        auto color_ = color(random_double(), random_double(), random_double());
        world.add(make_shared<quad>(point3(x1, y1, z1), vec3(x2 - x1, y2 - y1, z2 - z1), vec3(x3 - x1, y3 - y1, z3 - z1), make_shared<lambertian>(color_)));
    }

    camera cam;

    cam.aspect_ratio = settings.aspectRatio.value_or(1.0);
    cam.image_width = settings.imageWidth.value_or(400);
    cam.samples_per_pixel = settings.samplesPerPixel.value_or(10);
    cam.max_depth = settings.maxDepth.value_or(10);
    cam.background = settings.backgroundColor.has_value() ? color(static_cast<double>(settings.backgroundColor.value()[0])/ 255 , static_cast<double>(settings.backgroundColor.value()[1]) / 255 , static_cast<double>(settings.backgroundColor.value()[2]) / 255) : color(0, 0, 0);
    cam.vfov = settings.vfov.value_or(20);
    cam.lookfrom = settings.lookfrom.has_value() ? point3(settings.lookfrom.value()[0], settings.lookfrom.value()[1], settings.lookfrom.value()[2]) : point3(0, 0, 0);
    cam.lookat = settings.lookat.has_value() ? point3(settings.lookat.value()[0], settings.lookat.value()[1], settings.lookat.value()[2]) : point3(0, 0, 0);
    cam.vup = settings.vup.has_value() ? vec3(settings.vup.value()[0], settings.vup.value()[1], settings.vup.value()[2]) : vec3(0, 1, 0);

    cam.defocus_angle = settings.defocusAngle.value_or(0);
    cam.focus_dist = settings.focusDist.value_or(10);

    cam.render(world, [](int progress) {
        rendering_progress.store(progress);
    });
    
}

std::string clean_code(const std::string& raw_code) {
    std::string cleaned = raw_code;
    cleaned = std::regex_replace(cleaned, std::regex("/u003c"), "<");
    cleaned = std::regex_replace(cleaned, std::regex("/u003e"), ">");
    
    size_t pos = 0;
    while ((pos = cleaned.find("```cpp", pos)) != std::string::npos) {
        cleaned.erase(pos, 6); // Erase 3 characters at position 'pos'
    }
    pos = 0;
    while ((pos = cleaned.find(">>", pos)) != std::string::npos) {
        cleaned.replace(pos, 2, "> >"); // Replace ">>" with "> >"
        pos += 3;  // Move past the newly inserted "> >"
    }
    cleaned = std::regex_replace(cleaned, std::regex("```"), "");
    return cleaned;
}

void save_and_run_code(const std::string& code) {
    // Save to temporary file
    std::string temp_file = "temp_code.cpp";
    std::ofstream out(temp_file);
    out << code;
    out.close();
    
    // Compile
    std::string compile_cmd = "g++ -std=c++11 -o temp_program " + temp_file;
    int compile_result = system(compile_cmd.c_str());
    if (compile_result != 0) {
        std::cerr << "Compilation failed" << std::endl;
        return;
    }
    
    // Execute
    system("./temp_program");
    
    // Clean up
    remove(temp_file.c_str());
    remove("temp_program");
}

void render_scene(const CustomSettings& settings) {
    if (settings.prompt == "bouncing_spheres") {
        bouncing_spheres();
    } else if (settings.prompt == "checkered_spheres") {
        checkered_spheres();
    } else if (settings.prompt == "earth") {
        earth();
    } else if (settings.prompt == "perlin_spheres") {
        perlin_spheres();
    } else if (settings.prompt == "quads") {
        quads();
    } else if (settings.prompt == "simple_light") {
        simple_light();
    } else if (settings.prompt == "cornell_box") {
        cornell_box();
    } else if (settings.prompt == "custom") {
        custom_scene(settings);
    } else if (settings.prompt == "custom_ai") {
        std::string cleaned_code = clean_code(settings.response.value());
        save_and_run_code(cleaned_code);
    } else {
        throw std::invalid_argument("Invalid drawing option");
    }
    
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
                            <option value="custom">Custom Scenario</option>
                            <option value="custom_ai">Custom AI Prompt Scenario</option>
                        </select>
                    </div>
                    <div id="customSettings" class="custom-settings" style="display: none;">
                        <h2>Custom Scenario Settings</h2>
                        <div class="settings-grid">
                            <label for="aspectRatio">Aspect Ratio:</label>
                            <input type="number" id="aspectRatio" value="1.0" step="0.1">
                            
                            <label for="imageWidth">Image Width:</label>
                            <input type="number" id="imageWidth" value="100">
                            
                            <label for="samplesPerPixel">Samples Per Pixel:</label>
                            <input type="number" id="samplesPerPixel" value="10">
                            
                            <label for="maxDepth">Max Depth:</label>
                            <input type="number" id="maxDepth" value="10">
                            
                            <label for="backgroundColor">Background Color:</label>
                            <input type="color" id="backgroundColor" value="#000000">
                            
                            <label for="vfov">Vertical FOV:</label>
                            <input type="number" id="vfov" value="90">
                            
                            <label for="lookfromX">Look From (X, Y, Z):</label>
                            <div class="vector-input">
                                <input type="number" id="lookfromX" value="0">
                                <input type="number" id="lookfromY" value="0">
                                <input type="number" id="lookfromZ" value="0">
                            </div>
                            
                            <label for="lookatX">Look At (X, Y, Z):</label>
                            <div class="vector-input">
                                <input type="number" id="lookatX" value="0">
                                <input type="number" id="lookatY" value="0">
                                <input type="number" id="lookatZ" value="-1">
                            </div>
                            
                            <label for="vupX">View Up (X, Y, Z):</label>
                            <div class="vector-input">
                                <input type="number" id="vupX" value="0">
                                <input type="number" id="vupY" value="1">
                                <input type="number" id="vupZ" value="0">
                            </div>
                            
                            <label for="defocusAngle">Defocus Angle:</label>
                            <input type="number" id="defocusAngle" value="0">
                            
                            <label for="focusDist">Focus Distance:</label>
                            <input type="number" id="focusDist" value="10">
                            
                            <label for="numSpheres">Number of Spheres:</label>
                            <input type="number" id="numSpheres" value="0" min="0">
                            
                            <label for="numQuads">Number of Quadrilaterals:</label>
                            <input type="number" id="numQuads" value="0" min="0">
                        </div>
                    </div>
                    <div id="customSettingsAI" class="custom-settings-ai" style="display: none;">
                        <textarea id="aiInput" ></textarea>
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
                <script>
                    window.onload = function() {
                        document.getElementById('drawingOptions').addEventListener('change', function() {
                            var customSettings = document.getElementById('customSettings');
                            var customSettingsAI = document.getElementById('customSettingsAI');
                            if (this.value === 'custom_ai') {
                                customSettingsAI.style.display = 'block';
                            } else {
                                customSettingsAI.style.display = 'none';
                            }
                            if (this.value === 'custom') {
                                customSettings.style.display = 'block';
                            } else {
                                customSettings.style.display = 'none';
                            }
                        });
                    }
        
        
                    function renderScene() {
                        const selectedOption = document.getElementById("drawingOptions").value;
        
                        let sceneData = { prompt: selectedOption };
        
                        if (selectedOption === 'custom') {
                            sceneData.customSettings = {
                                aspectRatio: parseFloat(document.getElementById('aspectRatio').value),
                                imageWidth: parseInt(document.getElementById('imageWidth').value),
                                samplesPerPixel: parseInt(document.getElementById('samplesPerPixel').value),
                                maxDepth: parseInt(document.getElementById('maxDepth').value),
                                backgroundColor: document.getElementById('backgroundColor').value,
                                vfov: parseFloat(document.getElementById('vfov').value),
                                lookfrom: [
                                    parseFloat(document.getElementById('lookfromX').value),
                                    parseFloat(document.getElementById('lookfromY').value),
                                    parseFloat(document.getElementById('lookfromZ').value)
                                ],
                                lookat: [
                                    parseFloat(document.getElementById('lookatX').value),
                                    parseFloat(document.getElementById('lookatY').value),
                                    parseFloat(document.getElementById('lookatZ').value)
                                ],
                                vup: [
                                    parseFloat(document.getElementById('vupX').value),
                                    parseFloat(document.getElementById('vupY').value),
                                    parseFloat(document.getElementById('vupZ').value)
                                ],
                                defocusAngle: parseFloat(document.getElementById('defocusAngle').value),
                                focusDist: parseFloat(document.getElementById('focusDist').value),
                                numSpheres: parseInt(document.getElementById('numSpheres').value),
                                numQuads: parseInt(document.getElementById('numQuads').value)
                            };
                        }
                        if (selectedOption === 'custom_ai') { // Render AI image
                            const aiInputText = document.getElementById('aiInput').value;
                            sceneData = { prompt: aiInputText };
                            fetch("/renderAI", {
                                method: "POST",
                                headers: {
                                    "Content-Type": "application/json"
                                },
                                body: JSON.stringify(sceneData)
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
                        else { // Render custom or tutorial image
                            fetch("/render", {
                                method: "POST",
                                headers: {
                                    "Content-Type": "application/json"
                                },
                                body: JSON.stringify(sceneData)
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
                    }
                </script>
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

        crow::response res;
        res.set_static_file_info("user_image.jpg");
        res.set_header("Content-Type", "image/jpeg");
        return res;
    });
    
    CROW_ROUTE(app, "/renderAI").methods("POST"_method)
    ([](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);
        
        std::string prompt = x["prompt"].s();
        
        CustomSettings settings;
        settings.prompt = "custom_ai";
        try
        {
            httplib::Client cli("https://generativelanguage.googleapis.com");
            
            std::string json_payload = R"({
              "contents": [
                {
                  "role": "user",
                  "parts": [
                    {
                      "text": "Generate C++ code to create a ray traced image based on my raytracing library.\n\nExamples:\nCheckered Spheres:\nhittable_list world;\n\n    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));\n\n    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));\n    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));\n\n    camera cam;\n\n    cam.aspect_ratio      = 16.0 / 9.0;\n    cam.image_width       = 400;\n    cam.samples_per_pixel = 100;\n    cam.max_depth         = 50;\n    cam.background        = color(0.70, 0.80, 1.00);\n\n    cam.vfov     = 20;\n    cam.lookfrom = point3(13,2,3);\n    cam.lookat   = point3(0,0,0);\n    cam.vup      = vec3(0,1,0);\n\n    cam.defocus_angle = 0;\n\n    cam.render(world, [](int progress) {\n        rendering_progress.store(progress);\n    });\n\nBouncing Spheres:\nvoid bouncing_spheres() {\n    hittable_list world;\n    \n    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));\n    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));\n    \n    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));\n\n    for (int a = -11; a < 11; a++) {\n        for (int b = -11; b < 11; b++) {\n            auto choose_mat = random_double();\n            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());\n\n            if ((center - point3(4, 0.2, 0)).length() > 0.9) {\n                shared_ptr<material> sphere_material;\n\n                if (choose_mat < 0.8) {\n                    // diffuse\n                    auto albedo = color::random() * color::random();\n                    sphere_material = make_shared<lambertian>(albedo);\n                    auto center2 = center + vec3(0, random_double(0,.5), 0);\n                    world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));\n                } else if (choose_mat < 0.95) {\n                    // metal\n                    auto albedo = color::random(0.5, 1);\n                    auto fuzz = random_double(0, 0.5);\n                    sphere_material = make_shared<metal>(albedo, fuzz);\n                    world.add(make_shared<sphere>(center, 0.2, sphere_material));\n                } else {\n                    // glass\n                    sphere_material = make_shared<dielectric>(1.5);\n                    world.add(make_shared<sphere>(center, 0.2, sphere_material));\n                }\n            }\n        }\n    }\n\n    auto material1 = make_shared<dielectric>(1.5);\n    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));\n\n    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));\n    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));\n\n    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);\n    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));\n    \n    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));\n\n\n\n    camera cam;\n\n    cam.aspect_ratio      = 16.0 / 9.0;\n    cam.image_width       = 400;\n    cam.samples_per_pixel = 20;\n    cam.max_depth         = 20;\n    cam.background        = color(0.70, 0.80, 1.00);\n\n    cam.vfov     = 20;\n    cam.lookfrom = point3(13,2,3);\n    cam.lookat   = point3(0,0,0);\n    cam.vup      = vec3(0,1,0);\n\n    cam.defocus_angle = 0.6;\n    cam.focus_dist    = 10.0;\n\n    cam.render(world, [](int progress) {\n        rendering_progress.store(progress);\n    });\n\n}\n\nQuadrilaterals:\nvoid quads() {\n    hittable_list world;\n\n    // Materials\n    auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));\n    auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));\n    auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));\n    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));\n    auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));\n\n    // Quads\n    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));\n    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));\n    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));\n    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));\n    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));\n\n    camera cam;\n\n    cam.aspect_ratio      = 1.0;\n    cam.image_width       = 400;\n    cam.samples_per_pixel = 100;\n    cam.max_depth         = 50;\n    cam.background        = color(0.70, 0.80, 1.00);\n\n    cam.vfov     = 80;\n    cam.lookfrom = point3(0,0,9);\n    cam.lookat   = point3(0,0,0);\n    cam.vup      = vec3(0,1,0);\n\n    cam.defocus_angle = 0;\n\n    cam.render(world, [](int progress) {\n        rendering_progress.store(progress);\n    });\n    \n    // Store the rendered image\n    std::lock_guard<std::mutex> lock(image_mutex);\n    rendered_image = cam.image_buffer;\n}\n\nCornell Box:\nvoid cornell_box() {\n    hittable_list world;\n\n    auto red   = make_shared<lambertian>(color(.65, .05, .05));\n    auto white = make_shared<lambertian>(color(.73, .73, .73));\n    auto green = make_shared<lambertian>(color(.12, .45, .15));\n    auto light = make_shared<diffuse_light>(color(15, 15, 15));\n\n    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));\n    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));\n    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));\n    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));\n    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));\n    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));\n\n    camera cam;\n\n    cam.aspect_ratio      = 1.0;\n    cam.image_width       = 600;\n    cam.samples_per_pixel = 200;\n    cam.max_depth         = 50;\n    cam.background        = color(0,0,0);\n\n    cam.vfov     = 40;\n    cam.lookfrom = point3(278, 278, -800);\n    cam.lookat   = point3(278, 278, 0);\n    cam.vup      = vec3(0,1,0);\n\n    cam.defocus_angle = 0;\n\n    cam.render(world, [](int progress) {\n        rendering_progress.store(progress);\n    });\n    \n    // Store the rendered image\n    std::lock_guard<std::mutex> lock(image_mutex);\n    rendered_image = cam.image_buffer;\n}\n\nSimple Light:\nvoid simple_light() {\n    hittable_list world;\n\n    auto pertext = make_shared<noise_texture>(4);\n    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));\n    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));\n\n    auto difflight = make_shared<diffuse_light>(color(4,4,4));\n    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));\n    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));\n\n    camera cam;\n\n    cam.aspect_ratio      = 16.0 / 9.0;\n    cam.image_width       = 400;\n    cam.samples_per_pixel = 100;\n    cam.max_depth         = 50;\n    cam.background        = color(0,0,0);\n\n    cam.vfov     = 20;\n    cam.lookfrom = point3(26,3,6);\n    cam.lookat   = point3(0,2,0);\n    cam.vup      = vec3(0,1,0);\n\n    cam.defocus_angle = 0;\n\n    cam.render(world, [](int progress) {\n        rendering_progress.store(progress);\n    });\n    \n    // Store the rendered image\n    std::lock_guard<std::mutex> lock(image_mutex);\n    rendered_image = cam.image_buffer;\n}\n\nGenerate C++ code to produce an image according to the user input. \nFor the generation, include the whole file with all necessary includes, but omit all explanations and elaborations, we just want the cpp file. Assume all raytracing classes are ALREADY IMPLEMENTED and in the same directory. bvh.h,camera.h, constants.h, hittable.h,hittable_list.h, material.h, quad.h, sphere.h, texture.h. CONSTANTS.H BEFORE THE OTHERS SO NOTHING BREAKS. Just use them to draw. To avoid compile errors, also make sure you construct progress and image like std::atomic<int> rendering_progress(0); std::vector<unsigned char> rendered_image; Nothing else."
                    }
                  ]
                },
                {
                  "role": "model",
                  "parts": [
                    {
                      "text": "```cpp\nvoid glass_spheres() {\n    hittable_list world;\n\n    auto ground_material = make_shared<lambertian>(color(0.2, 0.2, 0.2));\n    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));\n\n    auto glass1 = make_shared<dielectric>(1.5);\n    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, glass1));\n    \n    auto glass2 = make_shared<dielectric>(1.5);\n    world.add(make_shared<sphere>(point3(-2, 1, 1), 1.0, glass2));\n\n    auto glass3 = make_shared<dielectric>(1.5);\n    world.add(make_shared<sphere>(point3(2, 1, -1), 1.0, glass3));\n\n    auto light = make_shared<diffuse_light>(color(1,1,1) * 0.5);\n    world.add(make_shared<sphere>(point3(0,5,0), 2, light));\n\n\n    camera cam;\n\n    cam.aspect_ratio      = 16.0 / 9.0;\n    cam.image_width       = 400;\n    cam.samples_per_pixel = 100;\n    cam.max_depth         = 50;\n    cam.background        = color(0.0,0.0,0.0);\n\n    cam.vfov     = 20;\n    cam.lookfrom = point3(10,3,5);\n    cam.lookat   = point3(0,0,0);\n    cam.vup      = vec3(0,1,0);\n\n    cam.defocus_angle = 0.0;\n\n    cam.render(world, [](int progress) {\n        rendering_progress.store(progress);\n    });\n    \n    std::lock_guard<std::mutex> lock(image_mutex);\n    rendered_image = cam.image_buffer;\n}\n```\n"
                    }
                  ]
                },
                {
                  "role": "user",
                  "parts": [
                    {
                      "text": ")" + prompt + R"("}
                  ]
                }
              ],
              "generationConfig": {
                "temperature": 1,
                "topK": 40,
                "topP": 0.95,
                "maxOutputTokens": 8192,
                "responseMimeType": "text/plain"
              }
            })";
            
            httplib::Headers headers = {
                {"Content-Type", "application/json"}
            };
            
            std::string path = "/v1beta/models/gemini-2.0-flash-exp:generateContent?key=AIzaSyAXI9unqZAdtHDF10FzvRdwS1e8mMFl8LE";
            
            auto res = cli.Post(path.c_str(), headers, json_payload, "application/json");
            
            if (res) {
                // Request successful
                std::cout << "Status: " << res->status << std::endl;
                std::cout << "Body: " << res->body << std::endl;
                
                crow::json::rvalue response_body = crow::json::load(res->body);
                settings.response = response_body["candidates"][0]["content"]["parts"][0]["text"].s();
                // Start the rendering in a separate thread
                std::thread(render_scene, settings).detach();
                return crow::response(200, "Rendering initiated");
            } else {
                // Error occurred
                std::cout << "Error: " << httplib::to_string(res.error()) << std::endl;
            }
            
        }
        catch (const std::exception& e)
        {
            std::cerr << "Request failed, error: " << e.what() << '\n';
        }
        return crow::response(500, "server error");
        
    });
     
    
    CROW_ROUTE(app, "/render").methods("POST"_method)
    ([](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);
        
        std::cout << x;

        std::string prompt = x["prompt"].s();

        CustomSettings settings;
        settings.prompt = prompt;

        // If custom settings are provided, parse them
        if (x.has("customSettings")) {
            auto& custom = x["customSettings"];
            if (custom.has("aspectRatio")) settings.aspectRatio = custom["aspectRatio"].d();
            if (custom.has("imageWidth")) settings.imageWidth = custom["imageWidth"].i();
            if (custom.has("samplesPerPixel")) settings.samplesPerPixel = custom["samplesPerPixel"].i();
            if (custom.has("maxDepth")) settings.maxDepth = custom["maxDepth"].i();
            if (custom.has("backgroundColor")) {
                auto& bg = custom["backgroundColor"];
                auto bgRGB = hexToRGB(bg.s());
                settings.backgroundColor = std::array<double, 3>{static_cast<double>(bgRGB.r), static_cast<double>(bgRGB.g), static_cast<double>(bgRGB.b)};
            }
            if (custom.has("vfov")) settings.vfov = custom["vfov"].d();
            if (custom.has("lookfrom")) {
                auto& lf = custom["lookfrom"];
                settings.lookfrom = std::array<double, 3>{lf[0].d(), lf[1].d(), lf[2].d()};
            }
            if (custom.has("lookat")) {
                auto& la = custom["lookat"];
                settings.lookat = std::array<double, 3>{la[0].d(), la[1].d(), la[2].d()};
            }
            if (custom.has("vup")) {
                auto& vup = custom["vup"];
                settings.vup = std::array<double, 3>{vup[0].d(), vup[1].d(), vup[2].d()};
            }
            if (custom.has("defocusAngle")) settings.defocusAngle = custom["defocusAngle"].d();
            if (custom.has("focusDist")) settings.focusDist = custom["focusDist"].d();
            if (custom.has("numSpheres")) settings.numSpheres = custom["numSpheres"].i();
            if (custom.has("numQuads")) settings.numQuads = custom["numQuads"].i();
        }

        // Start the rendering in a separate thread
        std::thread(render_scene, settings).detach();

        return crow::response(200, "Rendering initiated");
    });

    app.port(8080).multithreaded().run();
}
