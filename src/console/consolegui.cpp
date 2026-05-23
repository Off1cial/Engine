#include "console/consolegui.h"
#include "imgui/imgui.h"
#define CONSOLE_TEXT_DEFAULT_COL_IM (ImVec4(0.8, 0.8, 0.8, 1.0))
#define CONSOLE_TEXT_WARNING_COL_IM (ImVec4(0.8, 0.3, 0.3, 1.0))
#define CONSOLE_TEXT_INPUT_COL_IM (ImVec4(0.3, 0.3, 0.9, 1.0))

static int Console_TextEditCallback(ImGuiInputTextCallbackData *data)
{
  console_t *console = (console_t *)data->UserData;

  switch (data->EventFlag)
  {
  //
  // History navigation
  //
  case ImGuiInputTextFlags_CallbackHistory:
  {
    if (data->EventKey == ImGuiKey_UpArrow)
    {
      if (console->history_count > 0)
      {
        if (console->history_pos == -1)
          console->history_pos = console->history_count - 1;
        else if (console->history_pos > 0)
          console->history_pos--;

        data->DeleteChars(0, data->BufTextLen);

        data->InsertChars(
            0,
            console->history[console->history_pos]);
      }
    }
    else if (data->EventKey == ImGuiKey_DownArrow)
    {
      if (console->history_count > 0)
      {
        if (console->history_pos != -1)
        {
          console->history_pos++;

          if (console->history_pos >= console->history_count)
          {
            console->history_pos = -1;

            data->DeleteChars(0, data->BufTextLen);
          }
          else
          {
            data->DeleteChars(0, data->BufTextLen);

            data->InsertChars(
                0,
                console->history[console->history_pos]);
          }
        }
      }
    }

    break;
  }

  //
  // TAB autocomplete
  //
  case ImGuiInputTextFlags_CallbackCompletion:
  {
    const char *commands[] =
        {
            "help",
            "clear",
            "quit",
            "noclip",
            "god",
            "map",
            "sv_cheats",
            "r_wireframe"};

    const int num_commands =
        sizeof(commands) / sizeof(commands[0]);

    int match_count = 0;
    const char *match = NULL;

    for (int i = 0; i < num_commands; i++)
    {
      if (strncmp(
              commands[i],
              data->Buf,
              data->BufTextLen) == 0)
      {
        match_count++;
        match = commands[i];
      }
    }

    //
    // One exact match -> autocomplete
    //
    if (match_count == 1)
    {
      data->DeleteChars(0, data->BufTextLen);
      data->InsertChars(0, match);
    }
    //
    // Multiple matches -> print suggestions
    //
    else if (match_count > 1)
    {
      Console_WriteLine(
          "Possible matches:",
          CONSOLE_LINE_DEFAULT);

      for (int i = 0; i < num_commands; i++)
      {
        if (strncmp(
                commands[i],
                data->Buf,
                data->BufTextLen) == 0)
        {
          Console_WriteLine(
              commands[i],
              CONSOLE_LINE_DEFAULT);
        }
      }
    }

    break;
  }
  }

  return 0;
}

void ConsoleGUI_Draw()
{
  if (!gConsole->visible)
    return;

  ImGui::SetNextWindowSize(
      ImVec2(800, 400),
      ImGuiCond_FirstUseEver);

  ImGui::Begin(
      "Console",
      NULL,
      ImGuiWindowFlags_NoCollapse);

  //
  // Scrollback region
  //

  ImGui::BeginChild(
      "ScrollRegion",
      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
      ImGuiChildFlags_Borders);

  for (
      int i = gConsole->line_head;
      i != gConsole->line_tail;
      i = (i + 1) % CONSOLE_MAX_LINES)
  {
    console_line_t *line = &gConsole->lines[i];

    ImVec4 colour;

    switch (line->type)
    {
    case CONSOLE_LINE_WARNING:
      colour = ImVec4(0.8f, 0.3f, 0.3f, 1.0f);
      break;

    case CONSOLE_LINE_INPUT:
      colour = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);
      break;

    default:
      colour = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
      break;
    }

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        colour);

    ImGui::PushID(i);

    ImGui::TextUnformatted(
        line->text
        );
    ImGui::PopID();
    ImGui::PopStyleColor();
  }

  //
  // Auto-scroll
  //

  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();

  //
  // Input
  //

  ImGuiInputTextFlags flags =
      ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion;

  ImGui::SetNextItemWidth(-1);

  if (
      ImGui::InputText(
          "##ConsoleInput",
          gConsole->input,
          sizeof(gConsole->input),
          flags,
          Console_TextEditCallback,
          gConsole))
  {
    //
    // Execute command
    //

    if (gConsole->input[0] != '\0')
    {
      Console_WriteLine(
          gConsole->input,
          CONSOLE_LINE_INPUT);

      //
      // Store history
      //

      if (gConsole->history_count < CONSOLE_MAX_HISTORY)
      {
        strcpy(
            gConsole->history[gConsole->history_count],
            gConsole->input);

        gConsole->history_count++;
      }

      gConsole->history_pos = -1;

      //
      // TODO:
      // Console_Execute(gConsole->input);
      //

      gConsole->input[0] = '\0';
    }
  }

  //
  // Keep keyboard focus on input
  //

  ImGui::SetItemDefaultFocus();

  ImGui::End();
}