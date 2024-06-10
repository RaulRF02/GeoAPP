#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <libxml/HTMLparser.h>
#include <dirent.h>

std::vector<float> extract_energy_data(const std::string& html_file) {
    std::vector<float> consumption_data;
    htmlDocPtr doc = htmlReadFile(html_file.c_str(), nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == nullptr) {
        std::cerr << "Error al abrir el archivo HTML: " << html_file << std::endl;
        return consumption_data;
    }

    xmlNode* root_element = xmlDocGetRootElement(doc);
    if (!root_element) {
        std::cerr << "Error al obtener el elemento raíz del archivo HTML: " << html_file << std::endl;
        xmlFreeDoc(doc);
        return consumption_data;
    }

    xmlNode* cur_node = nullptr;
    for (cur_node = root_element; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && xmlStrcmp(cur_node->name, BAD_CAST "table") == 0) {
            xmlNode* tr_node = cur_node->children;
            while (tr_node) {
                if (tr_node->type == XML_ELEMENT_NODE && xmlStrcmp(tr_node->name, BAD_CAST "tr") == 0) {
                    xmlNode* td_node = tr_node->children;
                    int td_count = 0;
                    while (td_node) {
                        if (td_node->type == XML_ELEMENT_NODE && xmlStrcmp(td_node->name, BAD_CAST "td") == 0) {
                            if (td_count == 1) {
                                xmlChar* content = xmlNodeGetContent(td_node);
                                consumption_data.push_back(std::stof((const char*)content));
                                xmlFree(content);
                            }
                            td_count++;
                        }
                        td_node = td_node->next;
                    }
                }
                tr_node = tr_node->next;
            }
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return consumption_data;
}

float calculate_total(const std::vector<float>& data) {
    float total = 0;
    for (float value : data) {
        total += value;
    }
    return total;
}

void compare_energy_data(float before_total, float after_total) {
    std::cout << "Consumo total antes de la tarea: " << before_total << " W" << std::endl;
    std::cout << "Consumo total después de la tarea: " << after_total << " W" << std::endl;
    std::cout << "Diferencia de consumo: " << after_total - before_total << " W" << std::endl;
}

void calculate_cost_savings(float before_total, float after_total, float price_per_kwh_before, float price_per_kwh_after) {
    float time_hours = 10.0 / 3600.0; // 10 seconds in hours

    float energy_before_kwh = before_total * time_hours / 1000.0;
    float energy_after_kwh = after_total * time_hours / 1000.0;

    float cost_before = energy_before_kwh * price_per_kwh_before;
    float cost_after = energy_after_kwh * price_per_kwh_after;

    std::cout << "Costo antes de la tarea: " << cost_before << " €" << std::endl;
    std::cout << "Costo después de la tarea: " << cost_after << " €" << std::endl;
    std::cout << "Ahorro potencial: " << cost_before - cost_after << " €" << std::endl;
}

void analyze_single_client(const std::string& before_file, const std::string& after_file) {
    std::vector<float> before_data = extract_energy_data(before_file);
    std::vector<float> after_data = extract_energy_data(after_file);

    if (before_data.empty()) {
        std::cerr << "No se pudieron extraer datos del archivo: " << before_file << std::endl;
        return;
    }

    if (after_data.empty()) {
        std::cerr << "No se pudieron extraer datos del archivo: " << after_file << std::endl;
        return;
    }

    float before_total = calculate_total(before_data);
    float after_total = calculate_total(after_data);

    compare_energy_data(before_total, after_total);

    float price_per_kwh_before = 0.15;
    float price_per_kwh_after = 0.10;

    calculate_cost_savings(before_total, after_total, price_per_kwh_before, price_per_kwh_after);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <archivo_before_task.html> <archivo_after_task.html>" << std::endl;
        return 1;
    }

    std::string before_file = argv[1];
    std::string after_file = argv[2];

    analyze_single_client(before_file, after_file);

    return 0;
}
