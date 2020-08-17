#include <UI/UI.h>
#include <Core/Utils.h>
#include <Utility/stb_sprintf.h>

#include "Editor/Core/CoreUI.h"

bool entity_dropdown(const char *label, Entity *selected)
{
	static const char *unnamed = "Unnamed";
	bool changed = false;
	bool valid = selected->is_valid();
	uint32_t id = selected->id();
	const char *name = valid ? selected->name() ? selected->name() : unnamed : "None";

	if (ImGui::BeginCombo(label, name))
	{
		if (ImGui::Selectable("None", !valid) && valid)
		{ *selected = Entity::none; changed = true; }
		if (!valid) ImGui::SetItemDefaultFocus();

		for (uint32_t i(1); i <= Entity::entities.size; i++)
		{
			auto ent = Entity::get(i);
			if (ent == Entity::none)
				continue;

			ImGui::PushID(i);
			const bool is_selected = (valid && i == id);
			if (ImGui::Selectable(ent.name() ? ent.name() : unnamed, is_selected) && !is_selected)
			{ *selected = ent; changed = true; }
			if (is_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	return changed;
}

#ifdef PLATFORM_LINUX
#include <gtk/gtk.h>
#endif

bool choose_file(const char *name, char *output, size_t size, bool save)
{
	bool success = false;

#ifdef PLATFORM_LINUX
	if (!gtk_init_check(NULL, NULL))
		return false;

	auto action = save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget *dialog = gtk_file_chooser_dialog_new(name, NULL, action,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT,
		NULL);

	if (*output)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), output);
	else
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "Assets");
		if (save)
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "scene");
	}
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		stbsp_snprintf(output, size, "%s", filename);
		g_free(filename);
		success = true;
	}

	gtk_widget_destroy(dialog);
	while (gtk_events_pending())
		gtk_main_iteration();
#endif

	return success;
}
