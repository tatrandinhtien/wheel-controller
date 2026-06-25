# 4x4 Wheel Controller in Training Model

<div align="center">
  <video src="https://github.com/user-attachments/assets/7e0d2c64-697f-4408-bdc2-077278d0eeaf" controls width="480"></video>
</div>

## Table of Contents

| Section | Description |
|---|---|
| [1. Introduction](#1-introduction) | 4x4 Vehicle platform and wheel controller module overview. |
| [2. General Layout](#2-general-layout) |- Hardware.<br>- Software. |
| [3. Coding Rules](#3-coding-rules) | Define coding rules of the project. |

## 1. Introduction

This repository contains the high-performance embedded firmware designed for a distributed **4x4 Training Model Vehicle** platform. Featuring 4-Wheel Drive and 4-Wheel Steering (4WD/4WS) capabilities, this autonomous robotic platform allows independent torque and steering angle adjustments at each individual wheel assembly to achieve precise vector kinematics tracking.

This project was actively researched, developed, and validated within the **BOSCH Lab at Ho Chi Minh City University of Technology (HCMUT)** as a graduation thesis project. The primary engineering goal is to model, implement, and benchmark a rugged, real-time distributed automotive software architecture that complies with industrial embedded software production quality. 

This specific repository hosts the complete source code, bare-metal peripheral drivers, and real-time scheduling logic for the **Wheel Controller Module**—the localized intelligent edge node deployed at each of the four wheel configurations.

<center>
  <img src="docs/resources/images/hw_general_layout.png" alt="Hardware General Layout" width="600" />
</center>
<p align="center"><strong><em>Figure 1:</em></strong> Hardware General Layout of the Distributed System</p>

## 2. General Layout

### 2.1. Hardware

The Wheel Controller Module operates as a localized intelligent actuator node dedicated to each wheel assembly. 

Its core hardware execution tasks include driving the steering RC servo, regulating the DC bridge motor, decoding high-resolution quadrature encoder signals for closed-loop feedback, and maintaining robust communication across the shared bxCAN bus network.

<center>
  <img src="docs/resources/images/hardware_components.png" alt="Hardware Components Overview" width="600" />
</center>
<p align="center"><strong><em>Figure 2:</em></strong> Components Overview</p>

<center>
  <img src="docs/resources/images/wiring.png" alt="System Wiring Overview" width="600" />
</center>
<p align="center"><strong><em>Figure 3:</em></strong> Wiring Diagram</p>

To ensure high-reliability operational conditions and eliminate loose wiring clusters on the vehicle, we designed a customized, compact PCB layout. 

This dedicated board allows clean cable routing and enables seamless mechanical mounting straight onto each wheel segment of the training model framework.

<center>
  <img src="docs/resources/images/pcb_design.png" alt="Custom PCB Design View" width="600" />
</center>
<p align="center"><strong><em>Figure 4:</em></strong> PCB Design View  </p>

### 2.2. Software

## 3. Coding Rules