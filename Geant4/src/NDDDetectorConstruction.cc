#include "NDDDetectorConstruction.hh"
#include "NDDDetectorMessenger.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"

#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"

#include "G4UserLimits.hh"
#include "G4SystemOfUnits.hh"

NDDDetectorConstruction::NDDDetectorConstruction()
    : solidWorld(0),
      logicalWorld(0),
      physicalWorld(0),
      siThickness(2. * mm),
      siBackingThickness(3. * mm),
      siOuterRadius(7.5 * cm),
      deadLayerThickness(100. * nm),
      pixelRings(2),
      stepLimitMyl(0),
      stepLimitDead(0),
      stepLimitCar(0) {
  detMess = new NDDDetectorMessenger(this);
  detectorPosition = G4ThreeVector(0, 0, 10 * mm);
}

NDDDetectorConstruction::~NDDDetectorConstruction() {
  delete stepLimitMyl;
  delete stepLimitDead;
  delete stepLimitCar;
  delete detMess;
}

G4VPhysicalVolume* NDDDetectorConstruction::Construct() {

  BuildMaterials();
  BuildWorld();
  BuildSiDetector();
  BuildSources();
  BuildVisualisation();

  SetStepLimits();

  return physicalWorld;
}

void NDDDetectorConstruction::BuildMaterials() {
  // Material definitions
  G4double a, z;
  G4double density, pressure;
  G4int nel, natoms;

  // airMaterial at STP
  density = 1.293 * mg / cm3;
  G4Element* N = new G4Element("Nitrogen", "N", z = 7., a = 14.01 * g / mole);
  G4Element* O = new G4Element("Oxygen", "O", z = 8., a = 16.00 * g / mole);
  airMaterial = new G4Material("Air", density, nel = 2);
  airMaterial->AddElement(N, 70 * perCent);
  airMaterial->AddElement(O, 30 * perCent);

  // "Vacuum": 1e-6 Torr (low density air taken proportional to pressure)
  G4double densityAirSTP = density;
  pressure = 1e-8;  // Torr
  density = (pressure / 760.) * densityAirSTP;
  vacuumMaterial = new G4Material("Vacuum", density, nel = 2);
  vacuumMaterial->AddElement(N, 70 * perCent);
  vacuumMaterial->AddElement(O, 30 * perCent);

  // Silicon: properties from wikipedia
  siliconMaterial = new G4Material("Silicon", z = 14., a = 28.086 * g / mole,
                                   density = 2.329 * g / cm3);

  // copperMaterial: properties from wikipedia
  copperMaterial =
      new G4Material("copperMaterial", z = 29., a = 63.546 * g / mole,
                     density = 8.94 * g / cm3);

  // Stainless steel: model from
  // http://hypernews.slac.stanford.edu/HyperNews/geant4/get/geometry/915/1.html
  G4int nSS = 6;
  G4double fractionmass;
  density = 8.06 * g / cm3;
  stainlessSteelMaterial = new G4Material("SS", density, nSS);
  G4Element* C = new G4Element("Carbon", "C", z = 6., a = 12.011 * g / mole);
  G4Element* Si =
      new G4Element("Silicon", "Si", z = 14., a = 28.086 * g / mole);
  G4Element* Cr =
      new G4Element("Chromium", "Cr", z = 24., a = 51.996 * g / mole);
  G4Element* Mn =
      new G4Element("Manganese", "Mn", z = 25., a = 54.938 * g / mole);
  G4Element* Fe = new G4Element("Iron", "Fe", z = 26., a = 55.845 * g / mole);
  G4Element* Ni = new G4Element("Nickel", "Ni", z = 28., a = 58.693 * g / mole);
  stainlessSteelMaterial->AddElement(C, fractionmass = 0.001);
  stainlessSteelMaterial->AddElement(Si, fractionmass = 0.007);
  stainlessSteelMaterial->AddElement(Cr, fractionmass = 0.18);
  stainlessSteelMaterial->AddElement(Mn, fractionmass = 0.01);
  stainlessSteelMaterial->AddElement(Fe, fractionmass = 0.712);
  stainlessSteelMaterial->AddElement(Ni, fractionmass = 0.09);

  aluminiumMaterial = new G4Material("Aluminum", z = 13, a = 26.9815 * g / mole,
                                     density = 2.70 * g / cm3);

  tinMaterial = new G4Material("Tin", z = 50, a = 118.710 * g / mole,
                               density = 7.365 * g / cm3);

  bariumMaterial = new G4Material("Barium", z = 56, a = 137.327 * g / mole,
                                  density = 3510 * kg / m3);

  bismuthMaterial = new G4Material("Bismuth", z = 83, a = 208.980 * g / mole,
                                   density = 9.78 * g / cm3);

  G4Element* H = new G4Element("Hydrogen", "H", z = 1., a = 1.0008 * g / mole);
  mylarMaterial = new G4Material("Mylar", 1.370 * g / cm3, 3);
  mylarMaterial->AddElement(C, fractionmass = 0.62500);
  mylarMaterial->AddElement(H, fractionmass = 0.04167);
  mylarMaterial->AddElement(O, fractionmass = 0.33333);

  kaptonMaterial = new G4Material("Kapton", 1.42 * g / cm3, 4);
  kaptonMaterial->AddElement(N, fractionmass = 0.074);
  kaptonMaterial->AddElement(C, fractionmass = 0.691);
  kaptonMaterial->AddElement(O, fractionmass = 0.209);
  kaptonMaterial->AddElement(H, fractionmass = 0.026);

  pmmaMaterial = new G4Material("PMMA", 1.18 * g / cm3, 3);
  pmmaMaterial->AddElement(C, fractionmass = 0.6);
  pmmaMaterial->AddElement(H, fractionmass = 0.08);
  pmmaMaterial->AddElement(O, fractionmass = 0.32);

  sixFsixFMaterial = new G4Material("sixFsixF", 1.480 * g / cm3, 5);
  G4Element* F = new G4Element("Fluorine", "F", z = 9., a = 18.9984 * g / mole);
  sixFsixFMaterial->AddElement(H, fractionmass = 0.27027);
  sixFsixFMaterial->AddElement(C, fractionmass = 0.48648);
  sixFsixFMaterial->AddElement(N, fractionmass = 0.02703);
  sixFsixFMaterial->AddElement(O, fractionmass = 0.05405);
  sixFsixFMaterial->AddElement(F, fractionmass = 0.16216);

  aluminaMaterial = new G4Material("Alumina", 1.370 * g / cm3, 2);
  G4Element* Al =
      new G4Element("Aluminum", "Al", z = 13., a = 26.9815 * g / mole);
  aluminaMaterial->AddElement(Al, fractionmass = 0.53);
  aluminaMaterial->AddElement(O, fractionmass = 0.47);

  CaCl2Material = new G4Material("CaCl2", 2.15 * g / cm3, 2);
  G4Element* Ca = new G4Element("Calcium", "Ca", z = 20, a = 44.98 * g / mole);
  G4Element* Cl = new G4Element("Chlorine", "Cl", z = 17, a = 35.5 * g / mole);
  CaCl2Material->AddElement(Ca, fractionmass = 0.387);
  CaCl2Material->AddElement(Cl, fractionmass = 0.613);

  berylliumMaterial = new G4Material("Beryllium", z = 4, a = 9.0121 * g / mole,
                                     density = 1.85 * g / cm3);

  germaniumMaterial = new G4Material(
      "Germanium", z = 32., a = 72.630 * g / mole, density = 5.323 * g / cm3);

  waterMaterial = new G4Material("Water", 1.000 * g / cm3, 2);
  waterMaterial->AddElement(H, natoms = 2);
  waterMaterial->AddElement(O, natoms = 1);
  // Print all the material definitions
  G4cout << G4endl << "Material Definitions : " << G4endl << G4endl;
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

void NDDDetectorConstruction::BuildWorld() {
  // World volume
  G4double worldX = 0.5 * m;
  G4double worldY = 0.5 * m;
  G4double worldZ = 6.0 * m;
  solidWorld = new G4Box("World", worldX, worldY, worldZ);
  logicalWorld = new G4LogicalVolume(solidWorld, vacuumMaterial, "World");
  physicalWorld =
      new G4PVPlacement(0, G4ThreeVector(), logicalWorld, "World", 0, false, 0);
}

void NDDDetectorConstruction::BuildSiDetector() {
  G4double xDead = detectorPosition.x();
  G4double yDead = detectorPosition.y();
  G4double zDead = detectorPosition.z() + deadLayerThickness / 2.0;

  solidDead = new G4Tubs("solidDead", 0., siOuterRadius,
                         deadLayerThickness / 2., 0., 360. * deg);
  logicalDead = new G4LogicalVolume(solidDead, siliconMaterial, "Dead");
  physicalDead = new G4PVPlacement(
      0, G4ThreeVector(xDead, yDead, zDead), logicalDead,
      "Dead", logicalWorld, false, 0);

  G4double rotationAngleWest = 216 * deg;
  G4double rotationAngle = -257.5 * deg;

  G4RotationMatrix* rot = new G4RotationMatrix(rotationAngle, 0., 0.);
  G4RotationMatrix* rotWest = new G4RotationMatrix(rotationAngleWest, 0., 0.);

  // Simple model for Silicon active region
  G4double xSilicon = detectorPosition.x();
  G4double ySilicon = detectorPosition.y();
  G4double zSilicon = detectorPosition.z() +
                          deadLayerThickness + siThickness / 2.0;
  // G4cout << "!!!!!!!!!!!!!!!!!!!!!" << " " << zSilicon << G4endl;

  solidSilicon = new G4Tubs("solidSilicon", 0., siOuterRadius,
                                siThickness / 2., 0., 360. * deg);
  logicalSilicon = new G4LogicalVolume(solidSilicon, siliconMaterial,
                                           "logicalSilicon");
  physicalSilicon = new G4PVPlacement(
      rot,
      G4ThreeVector(xSilicon, ySilicon, zSilicon),
      logicalSilicon, "physicalSilicon", logicalWorld, false, 0);

  // Alumina backing (?)
  G4double zBack =
      zSilicon + siThickness / 2. + siBackingThickness / 2.;

  solidBacking = new G4Tubs("Backing", 0., siOuterRadius,
                            siBackingThickness / 2., 0., 360. * deg);
  logicalBacking =
      new G4LogicalVolume(solidBacking, aluminaMaterial, "Backing");
  physicalBacking = new G4PVPlacement(
      0, G4ThreeVector(xSilicon, ySilicon, zBack),
      logicalBacking, "Backing", logicalWorld, false, 0);
}

void NDDDetectorConstruction::BuildSources() {
  if (sourceIDs.size() != sourcePos.size()) {
    G4cout << "ERROR: incompatible amount of source IDs and corresponding positions. Not building sources." << G4endl;
  } else {
    for (G4int i = 0; i < sourceIDs.size(); i++) {
      BuildSource(sourceIDs[i], sourcePos[i]);
    }
  }
}

void NDDDetectorConstruction::BuildSource(G4int id, G4ThreeVector pos) {
  carrierRadius = 1.0 * mm;
  G4double foilRadius;
  G4double ringOuterRadius, ringInnerRadius;
  G4double eastFoilThickness, westFoilThickness;
  G4double ringThickness;

  G4LogicalVolume* motherVolume = logicalWorld;

  if (id == 0) {
    // 45Ca 500nm
    carrierThickness = 60 * nm;
    foilRadius = 2.3 * cm;
    eastFoilThickness = 500. * nm;
    ringOuterRadius = 2.54 / 2. * cm;
    ringInnerRadius = 2.3 / 2. * cm;
    ringThickness = 3.0 * mm;

    solidCarrier = new G4Tubs("Carrier", 0.0, carrierRadius,
                              carrierThickness / 2., 0.0, 360. * deg);
    logicalCarrier =
        new G4LogicalVolume(solidCarrier, CaCl2Material, "Carrier");
    physicalCarrier =
        new G4PVPlacement(0, G4ThreeVector(pos.x(), pos.y(), pos.z()),
                          logicalCarrier, "Carrier", motherVolume, false, 0);

    solidFoil = new G4Tubs("Foil", 0., foilRadius,
                               eastFoilThickness / 2., 0., 360. * deg);
    logicalFoil =
        new G4LogicalVolume(solidFoil, sixFsixFMaterial, "Foil");
    physicalFoil = new G4PVPlacement(
        0,
        G4ThreeVector(pos.x(), pos.y(),
                      pos.z() + (eastFoilThickness + carrierThickness) / 2.0),
        logicalFoil, "Foil", motherVolume, false, 0);

    solidSourceHolder =
        new G4Tubs("SourceHolder", ringInnerRadius, ringOuterRadius,
                   ringThickness / 2.0, 0.0, 360. * deg);
    logicalSourceHolder = new G4LogicalVolume(
        solidSourceHolder, aluminiumMaterial, "SourceHolder");
    physicalSourceHolder = new G4PVPlacement(
        0, G4ThreeVector(pos.x(), pos.y(), pos.z()), logicalSourceHolder,
        "SourceHolder", motherVolume, false, 0);
  } else if (id == 1) {
    // 133Ba 12.5um
    carrierThickness = 10.0 * nm;
    foilRadius = 11.0 * mm;
    eastFoilThickness = 10.0 * um;
    westFoilThickness = 12.5 * um;
    ringInnerRadius = 8.0 * mm;
    ringOuterRadius = 15.0 * mm;
    ringThickness = 1. * mm;

    solidFoil = new G4Tubs("Foil", 0., foilRadius,
                               eastFoilThickness / 2., 0, 360 * deg);
    logicalFoil =
        new G4LogicalVolume(solidFoil, mylarMaterial, "Foil");
    physicalFoil = new G4PVPlacement(
        0,
        G4ThreeVector(pos.x(), pos.y(),
                      pos.z() + (eastFoilThickness + carrierThickness) / 2.0),
        logicalFoil, "Foil", motherVolume, false, 0);

    // solidWestFoil = new G4Tubs("WestFoil", 0., foilRadius,
    //                            westFoilThickness / 2., 0, 360 * deg);
    // logicalWestFoil =
    //     new G4LogicalVolume(solidWestFoil, kaptonMaterial, "WestFoil");
    // physicalWestFoil = new G4PVPlacement(
    //     0,
    //     G4ThreeVector(pos.x(), pos.y(),
    //                   pos.z() - (westFoilThickness + carrierThickness) / 2.0),
    //     logicalWestFoil, "WestFoil", motherVolume, false, 0);

    solidCarrier = new G4Tubs("Carrier", 0.0, carrierRadius,
                              carrierThickness / 2.0, 0.0, 360. * deg);
    logicalCarrier =
        new G4LogicalVolume(solidCarrier, bariumMaterial, "Carrier");
    physicalCarrier =
        new G4PVPlacement(0, G4ThreeVector(pos.x(), pos.y(), pos.z()),
                          logicalCarrier, "Carrier", motherVolume, false, 0);

    solidSourceHolder =
        new G4Tubs("SourceHolder", ringInnerRadius, ringOuterRadius,
                   ringThickness / 2.0, 0, 360. * deg);
    logicalSourceHolder =
        new G4LogicalVolume(solidSourceHolder, pmmaMaterial, "SourceHolder");
    physicalSourceHolder = new G4PVPlacement(
        0, G4ThreeVector(pos.x(), pos.y(),
                         pos.z() + (carrierThickness + ringThickness) / 2.0 +
                             westFoilThickness),
        logicalSourceHolder, "SourceHolder", motherVolume, false, 0);
  } else if (id == 2) {
    // 133Ba 500nm, quasi sealed
  } else if (id == 3 || id == 4) {
    if (id == 3) {
      // 207Bi 5 um
      carrierThickness = 7. * um;
    } else {
      // 113Sn, 139Ce 5 um
      carrierThickness = 1. * nm;
    }
    foilRadius = 0.87 * 2.54 / 2.0 * cm;
    eastFoilThickness = 5.0 * um;
    westFoilThickness = 5.0 * um;
    ringInnerRadius = 0.87 * 2.54 / 2.0 * cm;
    ringOuterRadius = 2.54 / 2.0 * cm;
    ringThickness = 0.13 * 2.54 * cm;

    solidFoil = new G4Tubs("Foil", 0., foilRadius,
                               eastFoilThickness / 2.0, 0, 360. * deg);
    logicalFoil =
        new G4LogicalVolume(solidFoil, mylarMaterial, "Foil");
    physicalFoil = new G4PVPlacement(
        0,
        G4ThreeVector(pos.x(), pos.y(),
                      pos.z() + (carrierThickness + eastFoilThickness) / 2.0),
        logicalFoil, "Foil", motherVolume, false, 0);

    // solidWestFoil = new G4Tubs("WestFoil", 0., foilRadius,
    //                            westFoilThickness / 2.0, 0, 360. * deg);
    // logicalWestFoil =
    //     new G4LogicalVolume(solidWestFoil, mylarMaterial, "WestFoil");
    // physicalWestFoil = new G4PVPlacement(
    //     0, G4ThreeVector(pos.x(), pos.y(), pos.z() - (carrierThickness +
    //                                                   westFoilThickness) / 2.0),
    //     logicalWestFoil, "WestFoil", motherVolume, false, 0);

    solidCarrier = new G4Tubs("Carrier", 0.0, carrierRadius,
                              carrierThickness / 2.0, 0, 360. * deg);
    logicalCarrier =
        new G4LogicalVolume(solidCarrier, bismuthMaterial, "Carrier");
    physicalCarrier =
        new G4PVPlacement(0, G4ThreeVector(pos.x(), pos.y(), pos.z()),
                          logicalCarrier, "Carrier", motherVolume, false, 0);

    solidSourceHolder =
        new G4Tubs("SourceHolder", ringInnerRadius, ringOuterRadius,
                   ringThickness / 2.0, 0, 360. * deg);
    logicalSourceHolder = new G4LogicalVolume(
        solidSourceHolder, aluminiumMaterial, "SourceHolder");
    physicalSourceHolder = new G4PVPlacement(
        0, G4ThreeVector(pos.x(), pos.y(), pos.z()), logicalSourceHolder,
        "SourceHolder", motherVolume, false, 0);
  }
}

void NDDDetectorConstruction::BuildVisualisation() {
  // Visualization attributes

  logicalWorld->SetVisAttributes(G4VisAttributes::Invisible);

  G4VisAttributes* simpleBoxVisAttGreen =
      new G4VisAttributes(G4Colour(0.0, 1.0, 0.0));
  simpleBoxVisAttGreen->SetVisibility(true);
  simpleBoxVisAttGreen->SetForceSolid(false);

  logicalSilicon->SetVisAttributes(simpleBoxVisAttGreen);

  G4VisAttributes* simpleBoxVisAttRed =
      new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));
  simpleBoxVisAttRed->SetVisibility(true);
  simpleBoxVisAttRed->SetForceSolid(false);

  logicalDead->SetVisAttributes(simpleBoxVisAttRed);
}

void NDDDetectorConstruction::ConstructSDandField() {}

void NDDDetectorConstruction::SetStepLimits() {
  G4double maxStepDL = stepSize * deadLayerThickness;
  stepLimitDead = new G4UserLimits(maxStepDL);
  logicalDead->SetUserLimits(stepLimitDead);

  /*G4double maxStepWL = stepSize * waterThickness;
  G4UserLimits* stepLimitWater = new G4UserLimits(maxStepWL);
  logicalWater->SetUserLimits(stepLimitWater);

  G4double maxStepCar = stepSize * carrierThickness;
  stepLimitCar = new G4UserLimits(maxStepCar);
  logicalCarrier->SetUserLimits(stepLimitCar);

  G4double maxStepMyl = stepSize * mylarThickness;

  stepLimitMyl = new G4UserLimits(maxStepMyl);
  logicalFoil->SetUserLimits(stepLimitMyl);
  logicalWestFoil->SetUserLimits(stepLimitMyl);*/

  /*logicalWorld->SetUserLimits(
      new G4UserLimits(1e-3 * m, 1e2 * m, 1e3 * s, 1e-3 * keV));*/
}
