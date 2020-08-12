package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"net/http"

	"github.com/google/trillian/merkle"
	"github.com/google/trillian/merkle/rfc6962"
)

func main() {
	resp, err := http.Get("http://localhost:8080/verify")
	if err != nil {
		log.Fatal(err)
	}
	if resp.Body != nil {
		defer resp.Body.Close()
	}

	body, readErr := ioutil.ReadAll(resp.Body)
	if readErr != nil {
		log.Fatal(readErr)
	}

	mresp := MyResponse{}
	jsonErr := json.Unmarshal(body, &mresp)
	if jsonErr != nil {
		log.Fatal(jsonErr)
	}

	log.Printf("mresp: %v\n", mresp)

	v := merkle.NewLogVerifier(rfc6962.DefaultHasher)
	for i, proof := range mresp.Proof {
		hashes := proof.GetHashes()
		for j, hash := range hashes {
			log.Printf("[main] proof[%d],hash[%d] == %x\n", i, j, hash)
		}
		if err := v.VerifyInclusionProof(
			proof.LeafIndex,
			int64(mresp.Root.TreeSize),
			hashes,
			mresp.Root.RootHash,
			mresp.LeafHash,
		); err != nil {
			log.Println(err)
		} else {
			log.Println("[verification] Succeed.")
		}
	}
}

// mresp: {{1859867 [101 95 163 155 73 37 234 76 101 111 22 43 122 60 54 56 103 8 108 1 175 165 52 215 112 219 23 138 198 244 64 152] 1597243807574540312 69806 []} [leaf_index:1859866 hashes:"%kx\xa8\x14|\xc4\\=\xcf\x07Is\xbe\xf5\x91\x14BL\xfc\xa7\x9cZ\x88\xe1\xacI\xf2\xe1_>\x07" hashes:"'i&:\xc5\x03\xc5^B_=\xbc\x01,\xdf|[\x89\xee\xd2\xfe*\xcbJ\x176[Hƥ\xe3" hashes:"\x97K\xd9k5\x86\xd6\tyER\xfe?\xa4\x06\xd1,\x16\xbe\x10y\x02\x15\xa5P1w\xa2\xee\xb3+&" hashes:"\xed`-\x04\xbbH\xa3\xe9K\x06\xb0\xb7\x80\x16p\xe9\x06@\xc11\\\"q\xd8\x17\xd4G\xa5\xa7w\xb8\x15" hashes:"\x1a\n\x95\x87\xc5\xc9\xf9\x90Yd\xbd\xb4mc\xba\xf8\x9f@\xdd\xce\x1ec,BUx\x03I\x15Z\xd7\x1c" hashes:"Y\xd7\xd4J\x1eu\x92\xa0Cd\xae\xaa\xfd\x18\xd2\xdc\x06-\xbf\xa1\xe7\xd0\xfd\xd8\x0e\x80kC\x8bF\x92\xd1" hashes:"3P\xae\x85\xab݁\xc8\xe3±\x1dQAm0\xf5˔ͤ\x98\x99\x1bP\xf6`\x0b[\x9d\xa4\x06" hashes:"\xb1\xddk\x1e\x8d\xb9\x95zT\x91\x1f\xa3\x95$\xf3M\xb6!\xb2\xd8y6ӹ\xbc\xbc\xcb\xf7~@\xba\x83" hashes:"\x14\x1a\xad\xd1\x01,B\x8a.\x16\x12\xb1\xe1Mv\xb1\xd5U\xc1oޒ\xea\xfa\xc3\xeek\x94\xc9\xecQ\xf4"] [91 50 48 50 48 45 48 56 45 49 50 84 50 50 58 53 48 58 48 55 43 48 56 58 48 48 93 32 84 104 105 110 103] ok}
// resp] &{{1859867 [101 95 163 155 73 37 234 76 101 111 22 43 122 60 54 56 103 8 108 1 175 165 52 215 112 219 23 138 198 244 64 152] 1597243807574540312 69806 []} [leaf_index:1859866 hashes:"%kx\xa8\x14|\xc4\\=\xcf\x07Is\xbe\xf5\x91\x14BL\xfc\xa7\x9cZ\x88\xe1\xacI\xf2\xe1_>\x07" hashes:"'i&:\xc5\x03\xc5^B_=\xbc\x01,\xdf|[\x89\xee\xd2\xfe*\xcbJ\x176[Hƥ\xe3" hashes:"\x97K\xd9k5\x86\xd6\tyER\xfe?\xa4\x06\xd1,\x16\xbe\x10y\x02\x15\xa5P1w\xa2\xee\xb3+&" hashes:"\xed`-\x04\xbbH\xa3\xe9K\x06\xb0\xb7\x80\x16p\xe9\x06@\xc11\\\"q\xd8\x17\xd4G\xa5\xa7w\xb8\x15" hashes:"\x1a\n\x95\x87\xc5\xc9\xf9\x90Yd\xbd\xb4mc\xba\xf8\x9f@\xdd\xce\x1ec,BUx\x03I\x15Z\xd7\x1c" hashes:"Y\xd7\xd4J\x1eu\x92\xa0Cd\xae\xaa\xfd\x18\xd2\xdc\x06-\xbf\xa1\xe7\xd0\xfd\xd8\x0e\x80kC\x8bF\x92\xd1" hashes:"3P\xae\x85\xab݁\xc8\xe3±\x1dQAm0\xf5˔ͤ\x98\x99\x1bP\xf6`\x0b[\x9d\xa4\x06" hashes:"\xb1\xddk\x1e\x8d\xb9\x95zT\x91\x1f\xa3\x95$\xf3M\xb6!\xb2\xd8y6ӹ\xbc\xbc\xcb\xf7~@\xba\x83" hashes:"\x14\x1a\xad\xd1\x01,B\x8a.\x16\x12\xb1\xe1Mv\xb1\xd5U\xc1oޒ\xea\xfa\xc3\xeek\x94\xc9\xecQ\xf4"] [91 50 48 50 48 45 48 56 45 49 50 84 50 50 58 53 48 58 48 55 43 48 56 58 48 48 93 32 84 104 105 110 103] ok}
