package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"sync"
	"time"

	"github.com/google/trillian"

	"net/http"

	"google.golang.org/grpc"
)

var (
	tLogEndpoint = flag.String("tlog_endpoint", "3.237.199.254:8090", "The gRPC endpoint of the Trillian Log Server.")
	tLogID       = flag.Int64("tlog_id", 2093452198437103025, "Trillian Log ID")
)

func verifyHandler(w http.ResponseWriter, r *http.Request) {

	// Set variables
	// v, ok := r.URL.Query()["tlog_endpoint"]
	// if !ok || len(v[0]) < 1 {
	// 	log.Println("Url Param 'tlog_endpoint' is missing")
	// 	return
	// }
	// tLogEndpoint := v

	// v, ok = r.URL.Query()["tlog_id"]
	// if !ok || len(v[0]) < 1 {
	// 	log.Println("Url Param 'tlog_id' is missing")
	// 	return
	// }
	// tLogID, err := strconv.ParseInt(v[0], 10, 64)
	// if err == nil {
	// 	log.Printf("%d of type %T\n", tLogID, tLogID)
	// 	return
	// }

	log.Println("[main] Entered")

	// Establish gRPC connection w/ Trillian Log Server
	log.Printf("[main] Establishing connection w/ Trillian Log Server [%s]", *tLogEndpoint)
	conn, err := grpc.Dial(*tLogEndpoint, grpc.WithInsecure())
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	// Create a Trillian Log Server client
	log.Println("[main] Creating new Trillian Log Client")
	tLogClient := trillian.NewTrillianLogClient(conn)

	// Eventually this personality will be a server
	log.Printf("[main] Creating Server using LogID [%d]", *tLogID)
	server := newServer(tLogClient, *tLogID)

	// Leaves comprise a primary LeafValue (thing) and may have associated ExtraData(extra)
	// The LeafValue will become the hashed value for a node in the Merkle Tree
	log.Println("[main] Creating a 'Thing' and something 'Extra'")
	thing := newThing(fmt.Sprintf("[%s] Thing", time.Now().Format(time.RFC3339)))
	extra := newExtra("Extra")

	var wg sync.WaitGroup

	// Try to put this Request (Thing+Extra) in the Log
	log.Println("[main] Submitting it for inclusion in the Trillian Log")
	wg.Add(1)
	go func() {
		defer func() {
			log.Println("[main:put] Done")
			wg.Done()
		}()
		log.Println("[main:put] Entered")
		resp, err := server.put(&Request{
			thing: *thing,
			extra: *extra,
		})
		if err != nil {
			log.Fatal(err)
		}
		log.Printf("[main:put] Status:%s", resp.status)
	}()

	wg.Wait()

	// Await the Inclusion (Proof)
	log.Println("[main] Awaiting Inclusion (Proof) in the Trillian Log")
	// wg.Add(1)

	myResp := make(chan MyResponse)
	go func() {
		defer func() {
			log.Println("[main:wait] Done")
			// wg.Done()
		}()
		log.Println("[main:wait] Entered")
		for {
			resp, err := server.wait(&Request{
				thing: *thing,
			})
			if err != nil {
				log.Printf("[main:wait] %s", err)
			}
			log.Printf("[main:wait] Status:%s", resp.Status)
			if resp.Status == "ok" {
				myResp <- *resp
				log.Printf("[main:resp] %v\n", resp)
				break
			}
			log.Println("[main:wait] Sleeping")
			time.Sleep(1 * time.Second)
		}
	}()

	// wg.Wait()

	log.Println("[main] json Marshal")

	jsonResp, err := json.Marshal(<-myResp)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	log.Println("[main] Done")

	w.Header().Set("Content-Type", "application/json")
	w.Write(jsonResp)
}

func main() {
	// Start http server
	http.HandleFunc("/verify", verifyHandler)
	log.Println("[main] Open :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))

}
