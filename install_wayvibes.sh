#!/bin/env bash
# Wayvibes Auto-Installer - Downloads and installs wayvibes from GitHub

set -e

REPO_URL="https://github.com/sahaj-b/wayvibes"
INSTALL_DIR="$HOME/wayvibes"

RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

echo -e "${CYAN}--- Wayvibes Auto-Installer ---${RESET}"
echo -e "${CYAN}Downloading from: $REPO_URL${RESET}"

if ! command -v git &>/dev/null; then
  echo -e "${RED}❌ Git is not installed.${RESET}"
  exit 1
fi

if ! command -v make &>/dev/null; then
  echo -e "${RED}❌ Make is not installed.${RESET}"
  exit 1
fi

if [ -d "$INSTALL_DIR" ]; then
  echo -ne "\n${CYAN}Wayvibes directory already exists. Do you want to remove it and continue? (y/n): ${RESET}"
  read REMOVE_CHOICE
  if ! [[ "$REMOVE_CHOICE" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Exiting without changes.${RESET}"
    exit 0
  fi
  echo -e "${YELLOW}🗑️  Removing existing wayvibes installation...${RESET}"
  rm -rf "$INSTALL_DIR"
fi

echo -e "${CYAN}📥 Cloning wayvibes repository...${RESET}"
git clone "$REPO_URL" "$INSTALL_DIR"
cd "$INSTALL_DIR"

echo -e "${CYAN}🔨 Building with make...${RESET}"
make
if [ $? -ne 0 ]; then
  echo -e "${RED}❌ Build failed.${RESET}"
  exit 1
fi

echo -ne "\n${CYAN}Do you want to install wayvibes system-wide (sudo make install)? (y/n): ${RESET}"
read INSTALL_CHOICE
if [[ "$INSTALL_CHOICE" =~ ^[Yy]$ ]]; then
  sudo make install
  echo -e "${GREEN}✅ wayvibes installed to /usr/local/bin/${RESET}"
else
  echo -e "${YELLOW}You can run it from $INSTALL_DIR/wayvibes.${RESET}"
fi

echo -ne "\n${CYAN}Do you want to add wayvibes cmd to '~/.profile' for auto-starting? (y/n): ${RESET}"
read PROFILE_CHOICE
if [[ "$PROFILE_CHOICE" =~ ^[Yy]$ ]]; then

  # Try to find a soundpack directory (first subdir with config.json)
  SOUNDPACK_PATH=""
  for d in "$INSTALL_DIR"/*/; do
    if [ -f "$d/config.json" ]; then
      SOUNDPACK_PATH="$d"
      break
    fi
  done
  if [ -z "$SOUNDPACK_PATH" ]; then
    echo -e "${RED}Could not find a soundpack directory (with config.json) in $INSTALL_DIR.${RESET}"
    echo -ne "\n${CYAN}Please enter the full path to your soundpack: ${RESET}"
    read SOUNDPACK_PATH
    if [ ! -d "$SOUNDPACK_PATH" ] || [ ! -f "$SOUNDPACK_PATH/config.json" ]; then
      echo -e "${RED}❌ Invalid soundpack.${RESET}"
      echo -e "${RED}Wayvibes will not be added for auto-starting${RESET}"
      SOUNDPACK_PATH=""
    fi
  fi
  if [ -n "$SOUNDPACK_PATH" ]; then
    CMD="wayvibes \"$SOUNDPACK_PATH\" -v 3 -bg"
    if ! grep -Fxq "$CMD" ~/.profile; then
      echo "$CMD" >>~/.profile
      echo -en "${GREEN}✅ Added to ~/.profile:${RESET}"
      echo -e "${CYAN}$CMD${RESET}"
    else
      echo -e "${YELLOW}Command already present in ~/.profile${RESET}"
    fi
  fi
else
  echo -e "${YELLOW}Skipping auto-start setup...${RESET}"
fi

echo -e "${GREEN}\n🎉 Done! Wayvibes is ready to vibe!${RESET}"
echo -e "${CYAN}Run this command for usage: wayvibes --help${RESET}"
