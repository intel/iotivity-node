module.exports = function produceClientContent( item, creds ) {

	return {
		acl: {
			aclist2: [
				{
					aceid: 1,
					subject: { conntype: "anon-clear" },
					resources: [
						{ href: "/oic/res" },
						{ href: "/oic/d" },
						{ href: "/oic/p" },
						{ href: "/oic/sec/doxm" }
					],
					permission: 2
				},
				{
					aceid: 2,
					subject: { conntype: "auth-crypt" },
					resources: [
						{ href: "/oic/res" },
						{ href: "/oic/d" },
						{ href: "/oic/p" },
						{ href: "/oic/sec/doxm" }
					],
					permission: 2
				}
			],
			rowneruuid: item.deviceUuid
		},
		pstat: {
			dos: { s: 3, p: false },
			isop: true,
			rowneruuid: item.deviceUuid,
			cm: 0,
			tm: 0,
			om: 4,
			sm: 4
		},
		doxm: {
			oxms: [ 0 ],
			oxmsel: 0,
			sct: 1,
			owned: true,
			deviceuuid: item.deviceUuid,
			devowneruuid: item.deviceUuid,
			rowneruuid: item.deviceUuid
		},
		cred: {
			creds: creds
		}
	};
};
