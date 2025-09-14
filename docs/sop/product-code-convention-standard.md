# Product Code Convention Company Standard

**Document Type**: Company Standard  
**Scope**: Cannasol Technologies Products  
**Status**: Active  
**Version**: 1.0  
**Effective Date**: 2025-09-08  
**Author**: Stephen Boyett  
**Company**: Cannasol Technologies

## Software Products

TODO: Define software product code convention

### Software Product Code Format Breakdown

## Hardware Products

TODO: Define hardware product code convention

### Hadware Product Code Format Breakdown

## Liquid Products

TODO: Define liquid product code convention

### Liquid Product Code Format Breakdown

## TODO: Solid Products

TODO: Define solid product code convention

### Solid Product Code Format Breakdown

## Powder Products

TODO: Define powder product code convention

### Powder Product Code Format Breakdown

TODO: Define emulsion product code convention

## Emulsion Products

> **Product Code Format**: `CCPPNN1`

### Emulsion Product Code Format Breakdown

#### CC: Production Location

The first two characters indicate the production location.

- **CT**: In-house production (default)

#### PP: Product Type

The next two characters specify the product type. Examples include:

- **KV**: Kava product
- **D9**: Delta 9 THC
- **CN**: CBN
- **CG**: CBG
- **CD**: CBD

Additional product types may be defined as needed, following the same two-letter abbreviation standard.

#### NN: Potency (mg/ml or mg/g)

The following two characters represent the potency level of the product, measured in mg/ml (for liquids) or mg/g (for solids).

- Potency values should be numeric, e.g., **50** for 50 mg/ml or **30** for 30 mg/g.
- Ensure consistency in measurement units across similar product types.

#### 1: Surfactant/Formulation Type

The final character denotes the surfactant or formulation type used in the product.

- **N1**: NanoOptimizer™ (particle sizes ~30nm)
- **H1**: High-Loader™ (particle sizes 200-350nm)

### Example Generated Product Codes

Below is an example of generated product codes in JSON format, illustrating the convention in use:

```json
{
  "products": [
    {
      "timestamp": "2025-04-15T12:00:00Z",
      "productCode": "CTKV50H1",
      "productName": "Food-Grade Nano Kavalactone Powder",
      "productType": "emulstion",
      "revision": 1
    },
    {
      "timestamp": "2025-04-15T15:30:00Z",
      "productCode": "CTD910N1",
      "productName": "Food-Grade Delta-9 THC Nanoemulsion",
      "productType": "emulstion",
      "revision": 1
    },
    {
      "timestamp": "2025-04-15T14:30:00Z",
      "productCode": "CTCG30H1",
      "productName": "Food-Grade Cannabigerol Emulsion",
      "productType": "emulstion",
      "revision": 1
    }
  ]
}
```

This convention ensures consistent, readable, and scalable product identification across the Axovia Flow ecosystem.