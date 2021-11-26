<h1 align="center">
  <br>
  <a href="https://github.com/MakeMHz/xenium-fw-update"><img src="media/logo.png" alt="Xenium Firmware Update" width="800"></a>
  <br>
</h1>
<h4 align="center">Firmware update for XeniumOS used on Xenium and OpenXenium modchips to provide software fixes and various improvements.</h4>
<p align="center">
 <a href=""><img src="https://img.shields.io/discord/643467096906399804.svg" alt="Chat"></a>
 <a href="https://github.com/MakeMHz/xenium-fw-update/issues"><img src="https://img.shields.io/github/issues/MakeMHz/xenium-fw-update.svg" alt="GitHub Issues"></a>
 <a href=""><img src="https://img.shields.io/badge/contributions-welcome-orange.svg" alt="Contributions welcome"></a>
 <a href="https://opensource.org/licenses/GPL-2.0"><img src="https://img.shields.io/github/license/MakeMHz/xenium-fw-update.svg?color=green" alt="License"></a>
</p>
<p align="center">
  <a href="#about">About</a> •
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#licensing">Licensing</a>
</p>

## About
![screenshot](media/screenshot.png)

Released in early 2004, XeniumOS is the installed operating system on the Xenium series of modchips for the original Xbox. This OS provides users an interface for performing various actions such as flashing various BIOS images.

The xenium-fw-update project aims to fix long-standing issues since its release. Unfortunately, the source code for the XOS has never been released and this project is fully based on reverse-engineered binaries.

## Features
  * Fixes video output 1.6 revision systems when using component video.
  * Removes self-destruct logic. XOS performs device ID checks during boot, and if fails erases itself. In rare cases, this can happen on valid hardware.

## Installation
Download the latest release, copy xenium-os-update.xbe to your Xbox, and launch.

Note: Ensure that your Xenium Chip runs Firmware Version 2.3.1 before using the updater.

## Troubleshooting

- Question: Updater shows error message: Unknown XeniumOS version detected
- Answer: Update your Xenium Modchip with official Firmware 2.3.1, then run this tool again.

## Licensing
xenium-fw-update is free and open source. Please respect the licenses available in their respective folders.
