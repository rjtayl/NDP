# -*- coding: utf-8 -*-
"""
Created on Mon Dec  7 15:09:17 2020

@author: RJ
"""

import numpy as np
import unicodedata

def find_centers(N,R,s):    
    centers = [(0,0)]
    o_centers = [(0,0)]
    
    r = (np.sqrt(3)*R+s)
    theta = np.radians(np.arange(0,360,60))
    dx = r*np.cos(theta)
    dy = r*np.sin(theta)
    
    while len(centers) < N:
        new_centers = []
        for center in o_centers:
            oldx, oldy = center
            for i in range(len(theta)):
                new_center = (oldx+dx[i], oldy+dy[i])
                if new_center not in centers:
                    new_centers.append(new_center)
        centers = centers + new_centers
        o_centers = new_centers
    return centers[:N]

#determine how many rings needed to fit N pixels
def find_rings(N):
    rings= 0
    pixels = 1
    while pixels < N:
        rings += 1
        pixels = rings * 6 + pixels
    return rings

def tuple_average(tuple_list):
    x= 0
    y = 0
    n = len(tuple_list)
    for tup in tuple_list:
        x += tup[0]
        y += tup[1]
    return (x/n,y/n)

def max_dist(center, point_list):
    max_dist = 0
    for point in point_list:
        dist = np.sqrt(point[0]**2 + point[1]**2)
        if dist > max_dist:
            max_dist = dist
    return max_dist

#make bulk N type silicon tube large enough to fit pixels
def make_bulk(N, R, s, H):
    centers = find_centers(N,R,s)
    
    #place center in center of hexagon centers
    bulk_center = tuple_average(centers)
    
    #determine radius  
    L = max_dist(bulk_center, centers) + R + s + 2e-3
    
    bulk = ("SolidStateDetectors.Tube{T}"
            "(0.." + str(L) + ",0..2" + unicodedata.lookup("GREEK SMALL LETTER PI")+
            ",0.." + str(H) + ",CartesianPoint{T}(" + str(bulk_center[0]) + ", " +
            str(bulk_center[1]) + ", 0))")
    
    return bulk

#make p Si completely covering bottom of bulk Si
def make_pbottom(N, R, s, ph):
    centers = find_centers(N,R,s)
    
    #place center in center of hexagon centers
    bulk_center = tuple_average(centers)
    
    #determine radius  
    L = max_dist(bulk_center, centers) + R + s + 2e-3
    
        
    pcontact = ("SolidStateDetectors.Tube{T}"
                "(0.." + str(L) + ",0..2" + unicodedata.lookup("GREEK SMALL LETTER PI")+
                ",0.." + str(ph) + ",CartesianPoint{T}(" + str(bulk_center[0]) + 
                ", " + str(bulk_center[1]) + ", 0))")
    
    return pcontact

def make_pbottom_grad(N, R, s, ph, i, sigma, pC):
    centers = find_centers(N,R,s)
    
    #place center in center of hexagon centers
    bulk_center = tuple_average(centers)
    
    #determine radius  
    L = max_dist(bulk_center, centers) + R + s + 2e-3
    
        
    pcontact = ("G" + str(i) + " = SolidStateDetectors.Tube{T}"
                "(0.." + str(L) + ",0..2" + unicodedata.lookup("GREEK SMALL LETTER PI")+
                ",0.." + str(4*sigma) + ",CartesianPoint{T}(" + str(bulk_center[0]) + 
                ", " + str(bulk_center[1]) + ", " + str(ph+1e-9) + "))\n"
                "CD" + str(i) + "= SolidStateDetectors.LinearChargeDensity{T}"
                "((0.0f0,0.0f0," + str(pC) + "),(0.0f0,0.0f0,0.0f0))\n")
    
    return pcontact

#make a single pixel
def make_pixel(R, nh, center, n, nC, H):
    x,y = center
    pixel = ("G" + str(n) + " = SolidStateDetectors.HexagonalPrism{T}"
             "(0, " + str(R) + ", " + str(nh) + ",CartesianPoint{T}(" + str(x) + 
             ", " + str(y) + ", " + str(H-(nh/2)) +"),0)\n"
             "CD" + str(n) + "= SolidStateDetectors.LinearChargeDensity{T}"
             "((0.0f0,0.0f0," + str(nC) + "),(0.0f0,0.0f0,0.0f0))\n")
    return pixel

#make pspray or pstop around a single pixel
def make_pspray(R,s, tw, th, center, n,N,tC, H):
    x,y = center
    pixel = ("G" + str(n) + " = SolidStateDetectors.HexagonalPrism{T}"
             "(" + str(R+(s/2)-(tw/2)) + ", " + str(R+(s/2)) + ", " + str(th) + 
             ",CartesianPoint{T}(" + str(x) + ", " + str(y) + ", " + str(H-(th/2)) +"),0)\n"
             "CD" + str(n) + "= SolidStateDetectors.LinearChargeDensity{T}"
             "((0.0f0,0.0f0," + str(tC) + "),(0.0f0,0.0f0,0.0f0))\n")
    return pixel

#make N pixels starting with center and then moving counterclockwise around
def make_pixels(N,R,s,nh, nC, H, n0):    
    centers = find_centers(N,R,s)
    
    pixels = []
    for n in range(N):
        pixels.append(make_pixel(R,nh,centers[n],n+n0, nC, H))
    
    return pixels

def make_pixel_grad(R, nh, center, n, nC,H, sigma):
    x,y = center
    h=4*sigma
    pixel = ("G" + str(n) + " = SolidStateDetectors.HexagonalPrism{T}"
             "(0, " + str(R) + ", " + str(h) + ",CartesianPoint{T}(" + str(x) + 
             ", " + str(y) + ", " + str(H-nh-(h/2)) +"),0)\n"
             "CD" + str(n) + "= SolidStateDetectors.LinearChargeDensity{T}"
             "((0.0f0,0.0f0," + str(nC) + "),(0.0f0,0.0f0,0.0f0))\n")
    return pixel

def make_pixels_grad(N,R,s,nh, nC,H, n0, nstraggle):    
    centers = find_centers(N,R,s)
        
    pixels = []
    for n in range(N):
        pixels.append(make_pixel_grad(R,nh,centers[n],n+n0, nC,H, nstraggle))
    
    return pixels

#make pspray/pstop around every pixel
def make_psprays(N,R,s,tw, th, tC, H, n0):    
    centers = find_centers(N,R,s)
        
    pixels = []
    for n in range(N):
        pixels.append(make_pspray(R,s,tw,th,centers[n],n+n0,N, tC, H))
    
    return pixels

def make_pspray_grad(R,s, tw, th, center, n,N,tC,H, sigma):
    x,y = center
    h = 4*sigma
    pixel = ("G" + str(n) + " = SolidStateDetectors.HexagonalPrism{T}"
             "(" + str(R+(s/2)-(tw/2)) + ", " + str(R+(s/2)) + ", " + str(th) + 
             ",CartesianPoint{T}(" + str(x) + ", " + str(y) + ", " + str(H-th-(h/2)) +"),0)\n"
             "CD" + str(n) + "= SolidStateDetectors.LinearChargeDensity{T}"
             "((0.0f0,0.0f0," + str(tC) + "),(0.0f0,0.0f0,0.0f0))\n")
    return pixel

def make_psprays_grad(N,R,s,tw, th, tC,H, n0, psstraggle):    
    centers = find_centers(N,R,s)
        
    pixels = []
    for n in range(N):
        pixels.append(make_pspray_grad(R,s,tw,th,centers[n],n+n0,N, tC,H, psstraggle))
    
    return pixels


def main():
    ########### input parameters #######################
    name = input("Config file name: ")
    if name == "":
        name="unnamedIDP"
    
    N = input("Number of pixels: ")
    if N == "":
        N=1
    else:
        N = int(N.strip())
    
    H = input("Detector height(mm): ")
    if H == "":
        H=2e-3
    else:
        H = float(H.strip()) * 1e-3
        
    D = input("Pixel size(mm): ")
    if D == "":
        D=10.3e-3
    else:
        D = float(D.strip()) * 1e-3
    R = D/2
    
    s = input("Pixel seperation: ")
    if s == "":
        s=0.1e-3
    else:
        s = float(s.strip()) * 1e-3

    tw = input("pspray width: ")
    if tw == "":
        tw=0
    else:
        tw = float(tw.strip()) * 1e-3
    
    if tw > s:
        tw = input("pspray width was larger than seperation, try again: ")
        if tw == "":
            tw=0
        else:
            tw = float(tw.strip()) * 1e-3
    
    if tw > s:
        print("Still too large... we no longer take responsibility for what happens.")
            
    th = input("pspray thickness: ")
    if th == "":
        th=0
    else:
        th = float(th.strip()) * 1e-3
        
    tC = input("pspray impurity concentration: ")
    if tC == "":
        tC = -1.9e16
    else:
        tC = float(tC.strip()) * 1e-3
    
    ph = input("p+ thickness: ")
    if ph == "":
        ph = 0.01e-3
    else:
        ph = float(ph.strip()) * 1e-3
        
    pC = input("p+ impurity concentration: ")
    if pC == "":
        pC = -1.9e16
    else:
        pC = float(pC.strip()) * 1e-3
    
    nh = input("n+ thickness: ")
    if nh == "":
        nh = 0.02e-3
    else:
        nh = float(nh.strip()) * 1e-3
    
    nC = input("n+ impurity concentration: ")
    if nC == "":
        nC = 1.9e16
    else:
        nC = float(nC.strip()) * 1e9
        
    bulkC = input("Bulk impurity concentration: ")
    if bulkC == "":
        bulkC = 5.0e10
    else:
        bulkC = float(bulkC.strip()) * 1e9
    
    grad = input("IDP gradient(None, Gauss, Erf): ")
    if grad == "":
        grad = "None"
    if grad != "None":
        pstraggle = input("p+ straggle (sqrt(Dt)): ")
        if pstraggle == "":
            pstraggle = 0.037e-6
        else:
            pstraggle = float(pstraggle.strip()) * 1e-3
        nstraggle = input("n+ straggle: ")
        if nstraggle == "":
            nstraggle = np.sqrt(1e-14*3600*5)*1e-2
        else:
            nstraggle = float(nstraggle.strip()) * 1e-3
        if tw > 0 :
            psstraggle = input("pspray straggle: ")
            if psstraggle == "":
                psstraggle = 0.037e-6
            else:
                psstraggle = float(psstraggle.strip()) * 1e-3
        
    rad = input("Radial IDP gradient(None, Linear, Erf): ")
    if rad == "":
        rad = "None"
    if rad != "None":
        if rad == "Linear":
            slope = input("Slope of gradient: ")
            if slope == "":
                slope = 6e10
            else:
                slope = float(slope.strip()) * 1e12
        if rad == "Erf":
            stdev = input("Standard Deviation: ")
            if stdev == "":
                stdev = 10e-2
            else:
                stdev = float(stdev.strip()) * 1e-3
            height = input("Height: ")
            if height == "":
                height = 8e10
            else:
                height = float(height.strip())
            shift = input("Shift: ")
            if shift == "":
                shift = 70e-2
            else:
                shift = float(height.strip()) * 1e-3
        
     ################# Build CCD ######################   
        
    file_content= ("using SolidStateDetectors\n"
                   "using IntervalSets\n"
                   "using SpecialFunctions\n"
                   "T=Float32\n\n"
                   "struct CustomChargeDensity{T, N} <: SolidStateDetectors.AbstractChargeDensity{T}\n"
                   )
    
    #add bulk and p+ to CCD struct
    file_content += ("\tG1::SolidStateDetectors.AbstractGeometry{T,N}\n"
                     "\tG2::SolidStateDetectors.AbstractGeometry{T,N}\n"
                     "\tL1::SolidStateDetectors.AbstractChargeDensity{T}\n"
                     "\tL2::SolidStateDetectors.AbstractChargeDensity{T}\n")
    i = 2
    
    #add pixel to CCD struct
    pixels = []
    for n in range(N):
        i += 1
        pixels.append(i)
        file_content += ("\tG" + str(i) + "::SolidStateDetectors.AbstractGeometry{T,N}\n"
                         "\tL" + str(i) + "::SolidStateDetectors.AbstractChargeDensity{T}\n")
    
    #add pspray/pstop to CCD struct
        
    if tw > 0:
        pstops = []
        for n in range(N):
            i += 1
            pstops.append(i)
            file_content += ("\tG" + str(i) + "::SolidStateDetectors.AbstractGeometry{T,N}\n"
                             "\tL" + str(i) + "::SolidStateDetectors.AbstractChargeDensity{T}\n")
    
    if grad != "None":
        i+= 1
        pgrad = i
        file_content += ("\tG" + str(i) + "::SolidStateDetectors.AbstractGeometry{T,N}\n"
                         "\tL" + str(i) + "::SolidStateDetectors.AbstractChargeDensity{T}\n")
        pixelgrads = []
        for n in range(N):
            i += 1
            pixelgrads.append(i)
            file_content += ("\tG" + str(i) + "::SolidStateDetectors.AbstractGeometry{T,N}\n"
                             "\tL" + str(i) + "::SolidStateDetectors.AbstractChargeDensity{T}\n")
        pstopgrads = []
        if tw > 0:
            for n in range(N):
                i += 1
                pstopgrads.append(i)
                file_content += ("\tG" + str(i) + "::SolidStateDetectors.AbstractGeometry{T,N}\n"
                                 "\tL" + str(i) + "::SolidStateDetectors.AbstractChargeDensity{T}\n")
        
    file_content += "end\n\n"
    
    ######################## Build get_charge_density #####################
    
    file_content += "function SolidStateDetectors.get_charge_density(cdm::CustomChargeDensity{T}, pt::SolidStateDetectors.AbstractCoordinatePoint{T})::T where {T}\n"
    
    if rad == "Erf":
        file_content += ("\t(pt in cdm.G1) * SolidStateDetectors.get_charge_density(cdm.L1, pt)"
                         " + " + str(height) + " * (1 + erf((sqrt(pt[1]^2+pt[2]^2)-(" + str(shift) + "))/(2*" + str(stdev) + ")))/2 +\n")
                     
    else: file_content += "\t(pt in cdm.G1) * SolidStateDetectors.get_charge_density(cdm.L1, pt) +\n"
                     
    file_content += "\t(pt in cdm.G2) * SolidStateDetectors.get_charge_density(cdm.L2, pt) +\n"
    
    for n in pixels:
        file_content += "\t(pt in cdm.G" +str(n) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(n) + ", pt) +\n"
    
    if tw > 0 :
        for n in pstops:
            file_content += "\t(pt in cdm.G" +str(n) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(n) + ", pt) +\n"
    
    if grad == "Gauss":
        file_content += ("\t(pt in cdm.G" +str(pgrad) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(pgrad) + ", pt)"
                         " * exp(-(pt[3]-" + str(ph) + ")^2/(2*" + str(pstraggle) + "^2)) +\n")
        for n in pixelgrads:
            file_content += ("\t(pt in cdm.G" +str(n) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(n) + ", pt)"
                         " * exp(-(pt[3]-" + str(H-nh) + ")^2/(2*" + str(nstraggle) + "^2)) +\n")
        if tw > 0 :
            for n in pstopgrads:
                file_content += ("\t(pt in cdm.G" +str(n) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(n) + ", pt)"
                                 " * exp(-(pt[3]-" + str(H-th) + ")^2/(2*" + str(psstraggle) + "^2)) +\n")
    
    if grad == "Erf":
        file_content += ("\t(pt in cdm.G" +str(pgrad) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(pgrad) + ", pt)"
                         " * erfc(-(pt[3]-(" + str(ph+2*pstraggle) + "))/(2*" + str(pstraggle) + "))/2 +\n")
        for n in pixelgrads:
            file_content += ("\t(pt in cdm.G" +str(n) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(n) + ", pt)"
                             " * (1 + erf(-(pt[3]-(" + str(H-nh-(2*nstraggle)) + "))/(2*" + str(nstraggle) + ")))/2 +\n")
        if tw > 0 :
            for n in pstopgrads:
                file_content += ("\t(pt in cdm.G" +str(n) + ") * SolidStateDetectors.get_charge_density(cdm.L" +str(n) + ", pt)"
                                 " * (1 + erf(-(pt[3]-(" + str(H-th-(2*pstraggle)) + "))/(2*" + str(psstraggle) + ")))/2 +\n")  
    
    file_content = file_content[:-2]
    file_content += "\nend\n\n"
    
    ############## Build Geometries and Charge Distributions ################
    
    file_content += ("G1 = " + make_bulk(N,R,s,H) + "\n"
                     "G2 = " + make_pbottom(N,R,s,ph) + "\n")
    
    if rad == "Linear":
        file_content += "CD1 = SolidStateDetectors.CylindricalChargeDensity{T}((0.0f0,0.0f0," + str(bulkC) + "),(" + str(slope) +",0.0f0,0.0f0))\n"
    
    else: file_content += "CD1 = SolidStateDetectors.LinearChargeDensity{T}((0.0f0,0.0f0," + str(bulkC) + "),(0.0f0,0.0f0,0.0f0))\n"
    
    file_content += "CD2 = SolidStateDetectors.LinearChargeDensity{T}((0.0f0,0.0f0," + str(pC) + "),(0.0f0,0.0f0,0.0f0))\n"
    
    for pixel in make_pixels(N,R,s,nh,nC,H, pixels[0]):
        file_content += pixel
    
    if tw > 0 :
        for pixel in make_psprays(N,R,s,tw, th, tC,H, pstops[0]):
            file_content += pixel
    
    if grad != "None":
        file_content += make_pbottom_grad(N, R, s, ph, pgrad, pstraggle, pC)
        for pixel in make_pixels_grad(N,R,s,nh, nC,H, pixelgrads[0], nstraggle):
            file_content += pixel
        if tw > 0 :
            for pixel in make_psprays_grad(N,R,s,tw,th, nC,H, pstopgrads[0], psstraggle):
                file_content += pixel
                
    ###################### instantiate ccdm ##########################
    file_content += "\nccdm = CustomChargeDensity(G1,G2,CD1,CD2"
    
    for n in pixels:
        file_content += ",G" + str(n) + ",CD" + str(n)
        
    if tw > 0:
        for n in pstops:
            file_content += ",G" + str(n) + ",CD" + str(n)
            
    if grad != "None":
        file_content += ",G" + str(pgrad) + ",CD" + str(pgrad)
        for n in pixelgrads:
            file_content += ",G" + str(n) + ",CD" + str(n)
        if tw > 0 :
            for n in pstopgrads:
                file_content += ",G" + str(n) + ",CD" + str(n)
    
    file_content += ")\n"
    
    with open(name + ".jl", "w", encoding="utf-8") as outfile:
        outfile.write(file_content)
        
if __name__ == "__main__":
    main()
