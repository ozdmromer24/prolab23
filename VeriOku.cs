using Newtonsoft.Json;
using UlasimSistemi.Models;

namespace UlasimSistemi.Services
{
	public static class VeriOku
	{
		public static (List<Durak>, TaxiInfo) VeriYukle()
		{
			string json = File.ReadAllText("Data/veri.json");
			dynamic data = JsonConvert.DeserializeObject(json);

			var duraklar = new List<Durak>();

			foreach (var d in data.duraklar)
			{
				var durak = new Durak
				{
					Id = d.id,
					Name = d.name,
					Konum = new Konum { Latitude = d.lat, Longitude = d.lon },
					NextStops = new List<NextStop>()
				};

				foreach (var ns in d.nextStops)
				{
					durak.NextStops.Add(new NextStop
					{
						StopId = ns.stopId,
						Mesafe = ns.mesafe,
						Sure = ns.sure,
						Ucret = ns.ucret
					});
				}

				duraklar.Add(durak);
			}

			var taksi = new TaxiInfo
			{
				openingFee = data.taxi.openingFee,
				costPerKm = data.taxi.costPerKm
			};

			return (duraklar, taksi);
		}
	}
}
