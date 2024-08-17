#include <gtkmm.h>
#include <giomm.h>
#include <iostream>
#include <vector>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>

class NetworkManagerGUI : public Gtk::Window {
public:
    NetworkManagerGUI();

private:
    void on_scan_clicked();
    void on_connect_clicked(const std::string& ssid);
    void update_network_list();
    std::string exec(const char* cmd);
    std::string signal_to_bars(int signal);
    void apply_css();
    void connect_to_network(const std::string& ssid);
    std::string show_password_dialog(const std::string& ssid);
    bool network_requires_password(const std::string& ssid);
    std::string get_connected_network();
    void on_disconnect_clicked(const std::string& ssid);

    Gtk::Box m_vbox;
    Gtk::HeaderBar m_header_bar;
    Gtk::Button m_scan_button;
    Gtk::ScrolledWindow m_scrolled_window;
    Gtk::ListBox m_network_list;

    Glib::RefPtr<Gio::DBus::Proxy> m_nm_proxy;
};
NetworkManagerGUI::NetworkManagerGUI()
    : m_vbox(Gtk::ORIENTATION_VERTICAL),
      m_scan_button("Scan")
      //m_connect_button("Connect")
{
    set_title("WiFi Networks");
    set_default_size(400, 500);

    // Set up header bar
    m_header_bar.set_title("WiFi Networks");
    m_header_bar.set_subtitle("Available networks");
    m_header_bar.set_show_close_button(true);
    set_titlebar(m_header_bar);

    // Add scan button to header bar
    m_scan_button.get_style_context()->add_class("suggested-action");
    m_header_bar.pack_start(m_scan_button);

    add(m_vbox);

    // Set up network list
    m_scrolled_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    m_scrolled_window.add(m_network_list);
    m_vbox.pack_start(m_scrolled_window, Gtk::PACK_EXPAND_WIDGET);



    // Connect signals
    m_scan_button.signal_clicked().connect(sigc::mem_fun(*this, &NetworkManagerGUI::on_scan_clicked));

    // Set up D-Bus proxy
    try {
        m_nm_proxy = Gio::DBus::Proxy::create_for_bus_sync(
            Gio::DBus::BUS_TYPE_SYSTEM,
            "org.freedesktop.NetworkManager",
            "/org/freedesktop/NetworkManager",
            "org.freedesktop.NetworkManager"
        );
    } catch (const Glib::Error& ex) {
        std::cerr << "Error creating D-Bus proxy: " << ex.what() << std::endl;
    }

    apply_css();
    update_network_list();
    show_all_children();
}

void NetworkManagerGUI::on_scan_clicked() {
    std::cout << "Scanning for networks..." << std::endl;
    update_network_list();
}


void NetworkManagerGUI::on_connect_clicked(const std::string& ssid) {
    connect_to_network(ssid);
}


void NetworkManagerGUI::connect_to_network(const std::string& ssid) {
    try {
        std::cout << "Attempting to connect to: " << ssid << std::endl;

        // First attempt to connect without a password
        std::string command = "nmcli device wifi connect \"" + ssid + "\"";
        std::string result = exec(command.c_str());
        std::cout << result << std::endl;
        
        std::cout << "Initial connection attempt result: " << result << std::endl;

        // Check if connection failed due to missing credentials
        if (result.find("Error: Connection activation failed: Secrets were required, but not provided.") != std::string::npos) {
            std::cout << "Network requires password. Showing password dialog." << std::endl;
            std::string password = show_password_dialog(ssid);
            if (password.empty()) {
                std::cout << "Connection cancelled by user." << std::endl;
                return;
            }

            // Attempt to connect again with the provided password
            command = "nmcli device wifi connect \"" + ssid + "\" password \"" + password + "\"";
            result = exec(command.c_str());
            
            std::cout << "Connection attempt with password result: " << result << std::endl;
        }

        // Check if the connection was successful
        if (result.find("successfully activated") != std::string::npos) {
            update_network_list();
            std::cout << "Successfully connected to " << ssid << std::endl;
            Gtk::MessageDialog success_dialog(*this, "Connection Successful", false, Gtk::MESSAGE_INFO);
            success_dialog.set_secondary_text("Successfully connected to " + ssid);
            success_dialog.run();
        } else {
            std::cerr << "Failed to connect to " << ssid << ". Error: " << result << std::endl;
            Gtk::MessageDialog error_dialog(*this, "Connection Failed", false, Gtk::MESSAGE_ERROR);
            error_dialog.set_secondary_text(result);
            error_dialog.run();
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error connecting to network: " << e.what() << std::endl;
        Gtk::MessageDialog error_dialog(*this, "Connection Error", false, Gtk::MESSAGE_ERROR);
        error_dialog.set_secondary_text(e.what());
        error_dialog.run();
    }
}
std::string NetworkManagerGUI::show_password_dialog(const std::string& ssid) {
    Gtk::Dialog dialog("Enter Wi-Fi Password", *this, true);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Connect", Gtk::RESPONSE_OK);

    Gtk::Box* content_area = dialog.get_content_area();
    Gtk::Label label("Enter password for " + ssid + ":");
    content_area->pack_start(label);
    label.set_margin_bottom(10);

    Gtk::Entry password_entry;
    password_entry.set_visibility(false);  // Hide password characters
    password_entry.set_activates_default(true);  // Allow Enter key to submit
    content_area->pack_start(password_entry);
    password_entry.set_margin_bottom(10);

    dialog.set_default_response(Gtk::RESPONSE_OK);
    dialog.show_all_children();

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        return password_entry.get_text();
    }
    return "";
}

std::string NetworkManagerGUI::get_connected_network() {
    std::string result = exec("nmcli -t -f NAME,TYPE con show --active");
    std::istringstream iss(result);
    std::string line;

    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string type = line.substr(colon_pos + 1);
                return name;
        }
    }

    return "";  // Return empty string if no WiFi connection is active
}

void NetworkManagerGUI::on_disconnect_clicked(const std::string& ssid) {
    try {
        std::cout << "Disconnecting from: " << ssid << std::endl;
        std::string command = "nmcli connection down \"" + ssid + "\"";
        std::string result = exec(command.c_str());
        std::cout << result << std::endl;

        if (result.find("successfully deactivated") != std::string::npos) {
            std::cout << "Successfully disconnected from " << ssid << std::endl;
            Gtk::MessageDialog success_dialog(*this, "Disconnection Successful", false, Gtk::MESSAGE_INFO);
            success_dialog.set_secondary_text("Successfully disconnected from " + ssid);
            success_dialog.run();
            update_network_list();  // Refresh the network list
        } else {
            std::cerr << "Failed to disconnect from " << ssid << ". Error: " << result << std::endl;
            Gtk::MessageDialog error_dialog(*this, "Disconnection Failed", false, Gtk::MESSAGE_ERROR);
            error_dialog.set_secondary_text(result);
            error_dialog.run();
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error disconnecting from network: " << e.what() << std::endl;
        Gtk::MessageDialog error_dialog(*this, "Disconnection Error", false, Gtk::MESSAGE_ERROR);
        error_dialog.set_secondary_text(e.what());
        error_dialog.run();
    }
}


void NetworkManagerGUI::update_network_list() {
    // Clear existing list
    for (auto child : m_network_list.get_children()) {
        m_network_list.remove(*child);
    }

    // Get network list
    std::string result = exec("nmcli -t -f SSID,SIGNAL device wifi list");
    std::istringstream iss(result);
    std::string line;

    // Use a map to group networks by SSID
    std::map<std::string, std::vector<int>> networks;

    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string ssid = line.substr(0, colon_pos);
            int signal = std::stoi(line.substr(colon_pos + 1));
            
            networks[ssid].push_back(signal);
        }
    }

    // Get the currently connected network
    std::string connected_network = get_connected_network();

    // Create a vector of networks for sorting
    std::vector<std::pair<std::string, int>> sorted_networks;

    for (const auto& [ssid, signals] : networks) {
        int max_signal = *std::max_element(signals.begin(), signals.end());
        sorted_networks.emplace_back(ssid, max_signal);
    }

    // Sort networks by signal strength in descending order
    std::sort(sorted_networks.begin(), sorted_networks.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (const auto& [ssid, max_signal] : sorted_networks) {
        const auto& signals = networks[ssid];
        int ap_count = signals.size();
        
        auto* row = Gtk::make_managed<Gtk::ListBoxRow>();
        auto* hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 10);
        hbox->set_margin_top(5);
        hbox->set_margin_bottom(5);
        hbox->set_margin_start(10);
        hbox->set_margin_end(10);

        auto* ssid_label = Gtk::make_managed<Gtk::Label>(ssid);
        ssid_label->set_halign(Gtk::ALIGN_START);
        ssid_label->set_hexpand(true);

        hbox->pack_start(*ssid_label, Gtk::PACK_EXPAND_WIDGET);

        auto* signal_label = Gtk::make_managed<Gtk::Label>(signal_to_bars(max_signal));
        signal_label->get_style_context()->add_class("signal-strength");

        auto* ap_label = Gtk::make_managed<Gtk::Label>("(" + std::to_string(ap_count) + ")");
        ap_label->get_style_context()->add_class("ap-count");

        hbox->pack_start(*signal_label, Gtk::PACK_SHRINK);
        hbox->pack_start(*ap_label, Gtk::PACK_SHRINK);

        // Add "Connected" tag and disconnect button if this is the connected network
        if (ssid == connected_network) {
            auto* connected_label = Gtk::make_managed<Gtk::Label>("Connected");
            connected_label->get_style_context()->add_class("connected-tag");
            connected_label->set_margin_start(5);
            connected_label->set_margin_end(5);
            hbox->pack_start(*connected_label, Gtk::PACK_SHRINK);

            auto* disconnect_button = Gtk::make_managed<Gtk::Button>("Disconnect");
            disconnect_button->get_style_context()->add_class("disconnect-button");
            disconnect_button->signal_clicked().connect(
                sigc::bind(sigc::mem_fun(*this, &NetworkManagerGUI::on_disconnect_clicked), ssid));
            hbox->pack_start(*disconnect_button, Gtk::PACK_SHRINK);
        } else {
            // Add connect button for networks that are not currently connected
            auto* connect_button = Gtk::make_managed<Gtk::Button>("Connect");
            connect_button->get_style_context()->add_class("connect-button");
            connect_button->signal_clicked().connect(
                sigc::bind(sigc::mem_fun(*this, &NetworkManagerGUI::on_connect_clicked), ssid));
            hbox->pack_start(*connect_button, Gtk::PACK_SHRINK);
        }

        row->add(*hbox);
        m_network_list.append(*row);
    }

    show_all_children();
}

void NetworkManagerGUI::apply_css() {
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(R"(
        window {
            background-color: #f0f0f0;
        }
        headerbar {
            background: linear-gradient(to bottom, #4a90d9, #3670a9);
            color: white;
        }
        .signal-strength {
            font-family: monospace;
            font-weight: bold;
            color: #4a90d9;
        }
        .ap-count {
            color: #888;
            font-size: 0.8em;
        }
        .connected-tag {
            background-color: #4CAF50;
            color: white;
            padding: 2px 6px;
            border-radius: 3px;
            font-size: 0.8em;
            font-weight: bold;
        }
        .disconnect-button {
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 2px 6px;
            font-size: 0.8em;
        }
        .disconnect-button:hover {
            background-color: #d32f2f;
        }
        .connect-button {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 2px 6px;
            font-size: 0.8em;
        }
        .connect-button:hover {
            background-color: #45a049;
        }
        listbox {
            background-color: white;
            border-radius: 5px;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
        }
        listboxrow:selected {
            background-color: #4a90d9;
            color: white;
        }
        button {
            border-radius: 5px;
        }
        button.suggested-action {
            background-color: #4a90d9;
            color: white;
            border: none;
        }
    )");

    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

std::string NetworkManagerGUI::exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    int status = pclose(pipe);
    if (status == -1) {
        throw std::runtime_error("pclose() failed!");
    }
    // If the command failed, try to read from stderr
    if (result.empty() || status != 0) {
        pipe = popen((std::string(cmd) + " 2>&1").c_str(), "r");
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        result.clear();
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
    }
    return result;
}

std::string NetworkManagerGUI::signal_to_bars(int signal) {
    if (signal >= 80) return "▂▄▆█";
    if (signal >= 60) return "▂▄▆_";
    if (signal >= 40) return "▂▄__";
    if (signal >= 20) return "▂___";
    return "____";
}

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.example.networkmanagergui");

    NetworkManagerGUI window;

    return app->run(window);
}