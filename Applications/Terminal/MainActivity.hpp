
#pragma once

#include <Libc/stdlib.h>

#include <Libui/App/Activity.hpp>
#include <Libui/Button.hpp>
#include <Libui/EditText.hpp>
#include <Libui/LinearLayout.hpp>
#include <Libui/TextView.hpp>
#include <Libui/Events.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libgraphics/Font/FontLoader.hpp>

class MainActivity : public UI::Activity {
protected:
    void on_create() override
    {
        set_title("Terminal");

        constexpr uint32_t width = 460;
        constexpr uint32_t height = 340;

        auto font = new Graphics::BitmapFont(Graphics::FontLoader::load("/ext2/Resources/Roboto12Medium.fnt", "/ext2/Resources/Roboto12Medium.bmp"));

        auto layout = new UI::LinearLayout();
        auto layout_params = new UI::LayoutParams();
        layout_params->width = width;
        layout_params->height = height;
        layout->set_layout_params(layout_params);
        layout->set_orientation(UI::LinearLayout::Orientation::Vertical);

        static uint32_t counter = 0;
        static auto counter_view = new UI::TextView();
        counter_view->set_text("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum");
        counter_view->set_typeface(font);
        counter_view->set_background_color(Graphics::Color(240, 240, 240));
        auto counter_view_paprams = new UI::LayoutParams();
        counter_view_paprams->width = 460;
        counter_view_paprams->height = 320;

        int pty_master = open("/dev/ptmx", 1, 1);
        if (!pty_master) {
            Log << "could not open a ptmx device" << endl;
            exit(1);
        }

        UI::EventLoop::the().register_fd_for_select([&]() {
            char buff[255];
            int sz = read(pty_master, buff, sizeof(buff) - 1);
            buff[sz] = '\0';
            Log << "received from pty: " << String(buff) << endl;
        },
            pty_master);

        if (!fork()) {
            char* pty_slave_name = ptsname(pty_master);
            Log << "ptys name : " << pty_slave_name << endl;

            close(0);
            close(1);
            close(3);

            open(pty_slave_name, 1, 1);
            open(pty_slave_name, 1, 1);
            open(pty_slave_name, 1, 1);

            char* hello = "hello from pty!";
            write(0, hello, 16);

            exit(0);
        }

        auto edit_text = new UI::EditText();

        edit_text->set_text("> ");
        edit_text->set_typeface(font);
        edit_text->set_background_color(Graphics::Color(240, 240, 240));

        Function<void(const String& s, int start, int before, int count)> a = nullptr;

        a = [&](const String& s, int start, int before, int count) {
            Log << s[start] << endl;
        };

        edit_text->set_on_text_changed(a);

        auto edit_text_paprams = new UI::LayoutParams();
        edit_text_paprams->width = 460;
        edit_text_paprams->height = 20;

        layout->add_view(counter_view, counter_view_paprams);
        layout->add_view(edit_text, edit_text_paprams);

        set_content_view(layout);
    }
};